#pragma once
#include "broker_common.h"

namespace keryx {

class Broker;

class Consumer {
 public:
   Consumer(Broker &, StreamFilter const &, NotificationHandler const &);
   ~Consumer();
private:
   void on_notification(Notification const&);
   struct PImpl;
   std::unique_ptr<PImpl> me;
};

} // namespace keryx
