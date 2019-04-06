#include "Broker.h"
#include "../utils/keryx_assert.h"
#include "ConsumerImpl.h"
#include "ProducerImpl.h"
#include "Topic.h"
#include <atomic>
#include <inplace_function.h>

namespace keryx {

class StreamDescriptorRegistry {
 public:
   virtual StreamDescriptor const &get(StreamTypeID const &) = 0;
};

using Action = std::function<void(Broker &)>;

struct Broker::PImpl {
   PImpl(StreamDescriptorRegistry &reg, keryx_memory_resource &alloc)
       : producers(keryx_pmr<ProducerImplPtr>(&alloc)),
         consumers(keryx_pmr<ConsumerImplPtr>(&alloc)),
         stream_descriptor_registry(reg),
         pending_actions(keryx_pmr<Action>(&alloc)) {}

   keryx_vec<ProducerImplPtr> producers;
   keryx_vec<ConsumerImplPtr> consumers;
   StreamDescriptorRegistry &stream_descriptor_registry;
   keryx_queue<Action> pending_actions;
};

Broker::Broker(StreamDescriptorRegistry &r, keryx_memory_resource &alloc)
    : my_alloc(alloc),
      me(std::allocate_shared<PImpl>(keryx_pmr<PImpl>{&alloc}, r, alloc)) {}

Broker::~Broker() {}

ProducerImpl &Broker::make_producer(Topic const &topic,
                                    std::vector<EventPtr> const &snapshot) {
   auto p = std::allocate_shared<ProducerImpl>(
       keryx_pmr<ProducerImpl>{&my_alloc}, my_alloc,
       me->stream_descriptor_registry.get(topic.stream_type_id()), topic,
       snapshot);

   me->pending_actions.push([p](Broker &b) {
      b.me->producers.push_back(p);
      p->init(b.me->consumers);
   });

   return *p;
}

void Broker::publish(ProducerImpl &p, EventPtr const &ev) {
   me->pending_actions.push([&p, ev](Broker &) { p.publish(ev); });
}

void Broker::destroy_producer(ProducerImpl &p) {
   me->pending_actions.push([&p](Broker &b) {
      auto it = std::find_if(b.me->producers.begin(), b.me->producers.end(),
                             [&p](auto const &ptr) { return ptr.get() == &p; });
      keryx_assert(it != b.me->producers.end());
      b.me->producers.erase(it);
   });
}

ConsumerImpl &Broker::make_consumer(StreamFilter const &f,
                                    NotificationHandler const &h) {
   auto c =
       std::allocate_shared<ConsumerImpl>(keryx_pmr<ConsumerImpl>{&my_alloc});

   me->pending_actions.push([c, f, h](Broker &b) {
      b.me->consumers.emplace_back(c);
      *c = ConsumerImpl{f, h};
      for (auto &p : b.me->producers)
         p->maybe_add(*c);
   });
   return *c;
}

void Broker::destroy_consumer(ConsumerImpl &c) {
   me->pending_actions.push([&c](Broker &b) {
      for (auto &p : b.me->producers)
         p->maybe_remove(c);
      auto it = std::find_if(b.me->consumers.begin(), b.me->consumers.end(),
                             [&c](auto const &tc) { return tc.get() == &c; });
      b.me->consumers.erase(it);
   });
}

void Broker::do_work() {
   while (me->pending_actions.size()) {
      auto action = me->pending_actions.front();
      action(*this);
      me->pending_actions.pop();
   }
}

} // namespace keryx
