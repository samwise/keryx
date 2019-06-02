#include "OldBroker.h"
#include "../utils/keryx_assert.h"
#include "ConsumerImpl.h"
#include "ProducerImpl.h"
#include "TopicImpl.h"
#include <variant>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>

namespace keryx {

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

using keryx_action_queue =
    std::queue<Action, std::deque<Action, keryx_pmr<Action>>>;

void enqueue(keryx_action_queue &q, Action &&a);

struct OldBroker::PImpl {
   PImpl(keryx_memory_resource &alloc)
       : producers(keryx_pmr<ProducerImplPtr>(&alloc)),
         consumers(keryx_pmr<ConsumerImplPtr>(&alloc)),
         pending_actions(keryx_pmr<Action>{&alloc}) {}

   keryx_vec<ProducerImplPtr> producers;
   keryx_vec<ConsumerImplPtr> consumers;
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

OldBroker::OldBroker()
    : owned_alloc(new boost::container::pmr::unsynchronized_pool_resource()),
      my_alloc(*owned_alloc),
      me(keryx_allocate_unique<PImpl>(keryx_pmr<PImpl>{&my_alloc}, my_alloc)) {}

OldBroker::OldBroker(keryx_memory_resource &alloc)
    : owned_alloc(), my_alloc(alloc),
      me(keryx_allocate_unique<PImpl>(keryx_pmr<PImpl>{&my_alloc}, my_alloc)) {}

OldBroker::~OldBroker() {}

ProducerImpl &
OldBroker::make_producer(SnapshotHandlerPtr &&snapshot_handler,
                      TopicImpl const &topic,
                      keryx_vec<EventPtr> const &initial_snapshot) {
   auto p = keryx_allocate_unique<ProducerImpl>(
       keryx_pmr<ProducerImpl>{&my_alloc}, my_alloc,
       std::move(snapshot_handler), topic, initial_snapshot);
   auto p_ptr = p.get();
   enqueue(me->pending_actions, InitProducer{std::move(p)});
   return *p_ptr;
}

void OldBroker::publish(ProducerImpl &p, EventPtr &&ev) {
   enqueue(me->pending_actions, Publish{&p, std::move(ev)});
}

void OldBroker::destroy_producer(ProducerImpl &p) {
   enqueue(me->pending_actions, DestroyProducer{&p});
}

ConsumerImpl &OldBroker::make_consumer(StreamFilterImpl const &f,
                                    NotificationHandlerImpl const &h) {
   auto c = keryx_allocate_unique<ConsumerImpl>(
       keryx_pmr<ConsumerImpl>{&my_alloc}, f, h);
   auto c_ptr = c.get();
   enqueue(me->pending_actions, InitConsumer{std::move(c)});
   return *c_ptr;
}

void OldBroker::destroy_consumer(ConsumerImpl &c) {
   c.notify = [](auto const &) {};
   enqueue(me->pending_actions, DestroyConsumer{&c});
}

void OldBroker::do_work() {
   while (me->pending_actions.size()) {
      Action a = std::move(me->pending_actions.front());
      me->pending_actions.pop();
      std::visit(*me, a);
   }
}

void enqueue(keryx_action_queue &q, Action &&a) { q.push(std::move(a)); }

} // namespace keryx
