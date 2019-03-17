#include "Broker.h"
#include "../utils/keryx_assert.h"
#include "Consumer.h"
#include "Producer.h"
#include "Topic.h"

#include <atomic>

namespace keryx {

class ProducerTypeRegistry {
 public:
   virtual ProducerType const &get(ProducerTypeID const &) = 0;
};

struct Broker::PImpl {
   std::vector<std::unique_ptr<Producer>> producers;
   std::vector<std::unique_ptr<Consumer>> consumers;
   ProducerTypeRegistry &producer_type_registry;
};

Broker::Broker(ProducerTypeRegistry &r) : me(new PImpl{{},{},r}) {}
   Broker::~Broker() {}

   Producer &Broker::make_producer(Topic const &topic,
                                   std::vector<EventPtr> const &initial) {
      return *me->producers.emplace_back(
         new Producer{me->producer_type_registry.get(topic.producer_type_id()),
                       topic, initial, me->consumers});
   }

   void Broker::publish(Producer & p, EventPtr const &ev) { p.publish(ev); }

   void Broker::destroy_producer(Producer & p) {
      auto it = std::find_if(me->producers.begin(), me->producers.end(),
                             [&p](auto const &ptr) { return ptr.get() == &p; });
      keryx_assert(it != me->producers.end());
      me->producers.erase(it);
   }

   Consumer &Broker::make_consumer(ProducerFilter const &filter,
                                   NotificationHandler const &h) {
      auto &new_consumer = *me->consumers.emplace_back(new Consumer{filter, h});
      for (auto &p : me->producers)
         p->maybe_add(new_consumer);
      return new_consumer;
   }

   void Broker::destroy_consumer(Consumer & c) {
      for (auto &p : me->producers)
         p->maybe_remove(c);
   }

} // namespace keryx
