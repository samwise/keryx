#include "Broker.h"
#include "../utils/keryx_assert.h"
#include "ConsumerImpl.h"
#include "ProducerImpl.h"
#include "Topic.h"

#include <atomic>
#include <queue>

namespace keryx {

class ProducerTypeRegistry {
 public:
   virtual ProducerTypeDescriptor const &get(ProducerTypeID const &) = 0;
};

using Action = std::function<void(Broker &)>;

struct Broker::PImpl {
   std::vector<std::unique_ptr<ProducerImpl>> producers;
   std::vector<std::unique_ptr<ConsumerImpl>> consumers;
   ProducerTypeRegistry &producer_type_registry;
   std::queue<Action> pending_actions;
};

Broker::Broker(ProducerTypeRegistry &r) : me(new PImpl{{}, {}, r, {}}) {}
Broker::~Broker() {}

ProducerImpl &Broker::add_producer(ProducerImpl &p, Topic const &topic,
                                   std::vector<EventPtr> const &initial) {
   me->pending_actions.push([&p, topic, initial](Broker &b) {
      b.me->producers.emplace_back(&p);
      p.init(b.me->producer_type_registry.get(topic.producer_type_id()), topic,
             initial, b.me->consumers);
   });
   return p;
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

ConsumerImpl &Broker::add_consumer(ConsumerImpl &c, ProducerFilter const &f,
                                   NotificationHandler const &h) {
   me->pending_actions.push([&c, f, h](Broker &b) {
      b.me->consumers.emplace_back(&c);
      c = ConsumerImpl{f, h};
      for (auto &p : b.me->producers)
         p->maybe_add(c);
   });
   return c;
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
