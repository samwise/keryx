#include "Broker.h"
#include "../utils/keryx_assert.h"
#include "ConsumerImpl.h"
#include "ProducerImpl.h"
#include "Topic.h"

#include <atomic>
namespace keryx {

class ProducerTypeRegistry {
 public:
   virtual ProducerTypeDescriptor const &get(ProducerTypeID const &) = 0;
};

struct Broker::PImpl {
   std::vector<std::unique_ptr<ProducerImpl>> producers;
   std::vector<std::unique_ptr<ConsumerImpl>> consumers;
   ProducerTypeRegistry &producer_type_registry;
};

Broker::Broker(ProducerTypeRegistry &r) : me(new PImpl{{}, {}, r}) {}
Broker::~Broker() {}

ProducerImpl &Broker::add_producer(ProducerImpl &producer, Topic const &topic,
                                   std::vector<EventPtr> const &initial) {
   *me->producers.emplace_back(&producer);
   producer.init(me->producer_type_registry.get(topic.producer_type_id()),
                 topic, initial, me->consumers);
   return producer;
}

void Broker::publish(ProducerImpl &p, EventPtr const &ev) { p.publish(ev); }

void Broker::destroy_producer(ProducerImpl &p) {
   auto it = std::find_if(me->producers.begin(), me->producers.end(),
                          [&p](auto const &ptr) { return ptr.get() == &p; });
   keryx_assert(it != me->producers.end());
   me->producers.erase(it);
}

ConsumerImpl &Broker::add_consumer(ConsumerImpl &consumer,
                                   ProducerFilter const &filter,
                                   NotificationHandler const &h) {
   *me->consumers.emplace_back(&consumer);
   consumer = ConsumerImpl{filter, h};
   for (auto &p : me->producers)
      p->maybe_add(consumer);
   return consumer;
}

void Broker::destroy_consumer(ConsumerImpl &c) {
   for (auto &p : me->producers)
      p->maybe_remove(c);
   auto it = std::find_if(me->consumers.begin(), me->consumers.end(),
                          [&c](auto const &tc) { return tc.get() == &c; });
   me->consumers.erase(it);
}

} // namespace keryx
