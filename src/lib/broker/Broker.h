#pragma once
#include "broker_common.h"

namespace keryx {

class StreamDescriptorRegistry;

class Broker {
 public:
   Broker(StreamDescriptorRegistry &, keryx_memory_resource &alloc);
   ~Broker();

   ProducerImpl &make_producer(Topic const &,
                               std::vector<EventPtr> const &initial);
   void publish(ProducerImpl &, EventPtr const &);
   void destroy_producer(ProducerImpl &);

   ConsumerImpl &make_consumer(StreamFilter const &,
                               NotificationHandler const &);
   void destroy_consumer(ConsumerImpl &);
   void do_work();

 private:
   keryx_memory_resource &my_alloc;
   struct PImpl;
   // shared ptr because at this time there is no std implementation
   // of alloc_unique
   std::shared_ptr<PImpl> me;
};

} // namespace keryx
