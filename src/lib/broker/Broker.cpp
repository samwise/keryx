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

Broker::Broker(ProducerTypeRegistry &r) : me(new PImpl{{},{},r}) {}
   Broker::~Broker() {}

   ProducerImpl &Broker::make_producer(Topic const &topic,
                                   std::vector<EventPtr> const &initial) {
      return *me->producers.emplace_back(
         new ProducerImpl{me->producer_type_registry.get(topic.producer_type_id()),
                       topic, initial, me->consumers});
   }

   void Broker::publish(ProducerImpl & p, EventPtr const &ev) { p.publish(ev); }

   void Broker::destroy_producer(ProducerImpl & p) {
      auto it = std::find_if(me->producers.begin(), me->producers.end(),
                             [&p](auto const &ptr) { return ptr.get() == &p; });
      keryx_assert(it != me->producers.end());
      me->producers.erase(it);
   }

   ConsumerImpl &Broker::make_consumer(ProducerFilter const &filter,
                                   NotificationHandler const &h) {
      auto &new_consumer = *me->consumers.emplace_back(new ConsumerImpl{filter, h});
      for (auto &p : me->producers)
         p->maybe_add(new_consumer);
      return new_consumer;
   }

   void Broker::destroy_consumer(ConsumerImpl & c) {
      for (auto &p : me->producers)
         p->maybe_remove(c);
   }

} // namespace keryx
