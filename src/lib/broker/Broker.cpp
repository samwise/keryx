#include "Broker.h"
#include "../utils/keryx_assert.h"
#include "Producer.h"
#include "Consumer.h"

#include <atomic>

namespace keryx {

struct Broker::PImpl {
   std::vector<std::unique_ptr<Producer>> producers;
   std::vector<std::unique_ptr<Consumer>> consumers;
};

Broker::Broker() : me(new PImpl()) {}
Broker::~Broker() {}

Producer &Broker::make_producer(Topic const &topic,
                                IProducerTypeDescriptor &desc,
                                std::vector<EventPtr> const &initial) {
   return *me->producers.emplace_back(
       new Producer{topic, desc, initial, me->consumers});
}

void Broker::send_event(Producer &p, EventPtr const &ev) { p.send_event(ev); }

void Broker::destroy_producer(Producer &p) {
   auto it = std::find_if(me->producers.begin(), me->producers.end(),
                          [&p](auto const &ptr) { return ptr.get() == &p; });
   keryx_assert(it != me->producers.end());
   me->producers.erase(it);
}

Consumer &Broker::make_consumer(ProducerFilter const &filter,
                                MessageHandler const &h) {
   auto &new_consumer = *me->consumers.emplace_back(new Consumer{filter, h});
   for (auto &p : me->producers)
      p->maybe_add(new_consumer);
   return new_consumer;
}

void Broker::destroy_consumer(Consumer &c) {
   for (auto &p : me->producers)
      p->maybe_remove(c);
}

} // namespace keryx
