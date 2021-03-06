#pragma once

#include "SnapshotHandler.h"
#include "broker_common.h"

namespace keryx {

class StreamDescriptorRegistry;

class OldBroker {
 public:
   OldBroker();
   OldBroker(keryx_memory_resource &alloc);
   ~OldBroker();

   ProducerImpl &make_producer(SnapshotHandlerPtr &&, TopicImpl const &,
                               keryx_vec<EventPtr> const &initial_snapshot);
   void publish(ProducerImpl &, EventPtr &&);
   void destroy_producer(ProducerImpl &);

   ConsumerImpl &make_consumer(StreamFilterImpl const &,
                               NotificationHandlerImpl const &);
   void destroy_consumer(ConsumerImpl &);
   void do_work();
   keryx_memory_resource &alloc() {return my_alloc;}

 private:
   std::unique_ptr<keryx_memory_resource> owned_alloc;
   keryx_memory_resource &my_alloc;
   struct PImpl;
   keryx_unique_ptr<PImpl> me;
};

} // namespace keryx
