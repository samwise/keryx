#pragma once

#include "Broker.h"
#include "TopicImpl.h"

namespace keryx {

template <class Stream>
class Producer {
 public:
   using EventType = typename Stream::EventType;

   Producer(Broker &b,
            StreamName const&name = {},
            std::vector<EventType const*> const &initial_snapshot = {}) :
      broker(b),
      producer(broker.make_producer(Stream::make_snapshot_handler(b.alloc()),
                                    TopicImpl(typeid (Stream),name),
                                    convert_snapshot(initial_snapshot)))
   {
   }

   ~Producer() {broker.destroy_producer(producer);}

   void publish(const EventType &ev) {
      broker.publish(producer,Stream::clone_event(ev,broker.alloc()));
   }

 private:
   keryx_vec<EventPtr> convert_snapshot(std::vector<EventType const*> const&snapshot) {
      keryx_vec<EventPtr> result(keryx_pmr<EventPtr> {&broker.alloc()}) ;
      result.reserve(snapshot.size());
      for (auto const &ev : snapshot)
         result.push_back(Stream::clone_event(*ev,broker.alloc()));
      return result;
   }

   Broker &broker;
   ProducerImpl &producer;
};

} // namespace keryx
