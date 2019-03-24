#pragma once
#include "broker_common.h"

namespace keryx {

class QueuedBroker;

class Producer {
 public:
   Producer(QueuedBroker  &, Topic const &,
            std::vector<EventPtr> const &initial_snapshot = {});
   ~Producer();
   void publish(const Event &);
   
 private:
   struct PImpl;
   std::unique_ptr<PImpl> me;
};

} // namespace keryx
