#pragma once
#include "broker_common.h"

namespace keryx {
class Producer {
 public:
   Producer(Topic const &t, IProducerTypeDescriptor &desc,
            std::vector<EventPtr> const &initial_snapshot,
            std::vector<std::unique_ptr<Consumer>> const &consumers);

   ~Producer() ;
   void maybe_add(Consumer &) ;
   void maybe_remove(Consumer &);
   void send_event(EventPtr const &);

 private:
   bool is_match(ProducerFilter const &);
   struct PImpl;
   std::unique_ptr<PImpl> me;
};

}
