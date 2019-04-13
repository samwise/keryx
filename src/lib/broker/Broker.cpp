#include "Broker.h"
#include "../utils/keryx_assert.h"
#include "ConsumerImpl.h"
#include "ProducerImpl.h"
#include "Topic.h"
#include <atomic>
#include <inplace_function.h>
#include <variant>
#include <boost/circular_buffer.hpp>

namespace keryx {

class StreamDescriptorRegistry {
 public:
   virtual StreamDescriptor const &get(StreamType const &) = 0;
};

struct InitProducer {
   ProducerImplPtr producer;
};

struct DestroyProducer {
   ProducerImpl *producer;
};

struct Publish {
   ProducerImpl *producer;
   EventPtr event;
};

struct InitConsumer {
   ConsumerImplPtr consumer;
};

struct DestroyConsumer {
   ConsumerImpl *consumer;
};

using Action = std::variant<InitProducer, DestroyProducer, Publish,
                            InitConsumer, DestroyConsumer>;

// using keryx_action_queue = boost::circular_buffer<Action,keryx_pmr<Action>> ;
using keryx_action_queue = std::queue<Action,std::deque<Action,keryx_pmr<Action>>> ;

void enqueue(keryx_action_queue &q,Action &&a);
std::pair<bool,Action> dequeue(keryx_action_queue &q);

struct Broker::PImpl {
   PImpl(keryx_memory_resource &alloc,StreamDescriptorRegistry &reg)
       : producers(keryx_pmr<ProducerImplPtr>(&alloc)),
         consumers(keryx_pmr<ConsumerImplPtr>(&alloc)),
         stream_descriptor_registry(reg),
         pending_actions(keryx_pmr<Action> {&alloc}) {}

   keryx_vec<ProducerImplPtr> producers;
   keryx_vec<ConsumerImplPtr> consumers;
   StreamDescriptorRegistry &stream_descriptor_registry;
   keryx_action_queue pending_actions;

   void operator()(InitProducer &ip) {
      auto &p = *ip.producer;
      producers.push_back(std::move(ip.producer));
      p.init(consumers);
   }

   void operator()(Publish &p) { p.producer->publish(std::move(p.event)); }

   void operator()(DestroyProducer &dp) {
      auto it = std::find_if(
          producers.begin(), producers.end(),
          [&dp](auto const &ptr) { return ptr.get() == dp.producer; });
      keryx_assert(it != producers.end());
      producers.erase(it);
   }

   void operator()(InitConsumer &ic) {
      auto &c = *ic.consumer;
      consumers.emplace_back(std::move(ic.consumer));
      for (auto &p : producers)
         p->maybe_add(c);
   }

   void operator()(DestroyConsumer &dc) {
      for (auto &p : producers)
         p->maybe_remove(*dc.consumer);
      auto it = std::find_if(
          consumers.begin(), consumers.end(),
          [&dc](auto const &tc) { return tc.get() == dc.consumer; });
      consumers.erase(it);
   }
};

Broker::Broker(StreamDescriptorRegistry &r, keryx_memory_resource &alloc)
    : my_alloc(alloc),
      me(keryx_allocate_unique<PImpl>(keryx_pmr<PImpl>{&alloc},alloc, r)) {}

Broker::~Broker() {}

ProducerImpl &
Broker::make_producer(Topic const &topic,
                      std::vector<Event const *> const &snapshot) {
   auto p = keryx_allocate_unique<ProducerImpl>(
       keryx_pmr<ProducerImpl>{&my_alloc}, my_alloc,
       me->stream_descriptor_registry.get(topic.stream_type_id()), topic,
       snapshot);
   auto p_ptr = p.get();
   enqueue(me->pending_actions,InitProducer{std::move(p)});
   return *p_ptr;
}


void Broker::publish(ProducerImpl &p, Event const &ev) {
   enqueue(me->pending_actions,Publish{&p, p.clone_event(ev)});
}

void Broker::destroy_producer(ProducerImpl &p) {
   enqueue(me->pending_actions,DestroyProducer{&p});
}

ConsumerImpl &Broker::make_consumer(StreamFilter const &f,
                                    NotificationHandler const &h) {
   auto c = keryx_allocate_unique<ConsumerImpl>(
       keryx_pmr<ConsumerImpl>{&my_alloc}, f, h);
   auto c_ptr = c.get();
   enqueue(me->pending_actions,InitConsumer{std::move(c)});
   return *c_ptr;
}

void Broker::destroy_consumer(ConsumerImpl &c) {
   enqueue(me->pending_actions,DestroyConsumer{&c});
}

void Broker::do_work() {
   while (me->pending_actions.size())  {
      Action a = std::move(me->pending_actions.front());
      me->pending_actions.pop();
      std::visit(*me,a);
   }
}

void enqueue(keryx_action_queue &q,Action &&a)
{
   q.push(std::move(a));
   
   // if (q.full()) 
   //    q.set_capacity(q.capacity()*2);
   // q.push_back(std::move(a));
}

} // namespace keryx
