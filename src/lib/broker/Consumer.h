#pragma once
#include "broker_common.h"

namespace keryx {

class QueuedBroker;

class Consumer {
 public:
   Consumer(QueuedBroker &, ProducerFilter const &, NotificationHandler const &);
   ~Consumer();
private:
   void on_notification(Notification const&);
   struct PImpl;
   std::unique_ptr<PImpl> me;
};

} // namespace keryx
