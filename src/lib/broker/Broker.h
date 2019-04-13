#pragma once
#include "broker_common.h"

namespace keryx {

class StreamDescriptorRegistry;

class Broker {
 public:
   Broker(StreamDescriptorRegistry &, keryx_memory_resource &alloc);
   ~Broker();

   ProducerImpl &make_producer(Topic const &,
                               std::vector<Event const *> const &initial);
   void publish(ProducerImpl &, Event const &);
   void destroy_producer(ProducerImpl &);

   ConsumerImpl &make_consumer(StreamFilter const &,
                               NotificationHandler const &);
   void destroy_consumer(ConsumerImpl &);
   void do_work();

 private:
   keryx_memory_resource &my_alloc;
   struct PImpl;
   keryx_unique_ptr<PImpl> me;
};

} // namespace keryx
