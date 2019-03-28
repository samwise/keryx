#pragma once
#include "broker_common.h"

namespace keryx {

class ProducerTypeRegistry;

class Broker {
 public:
   Broker(ProducerTypeRegistry &);
   ~Broker();

   ProducerImpl &add_producer(ProducerImpl &, Topic const &,
                              std::vector<EventPtr> const &initial);
   void publish(ProducerImpl &, EventPtr const &);
   void destroy_producer(ProducerImpl &);

   ConsumerImpl &add_consumer(ConsumerImpl &, ProducerFilter const &,
                              NotificationHandler const &);
   void destroy_consumer(ConsumerImpl &);
   void do_work();
 private:
   struct PImpl;
   std::unique_ptr<PImpl> me;
};

} // namespace keryx
