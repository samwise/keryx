#pragma once
#include "broker_common.h"

namespace keryx {

class Broker;

class Producer {
 public:
   Producer(Broker &, Topic const &,
            std::vector<Event const*> const &initial_snapshot = {});
   ~Producer();
   void publish(const Event &);

 private:
   struct PImpl;
   std::unique_ptr<PImpl> me;
};

} // namespace keryx
