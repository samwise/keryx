#pragma once
#include "broker_common.h"
#include "SnapshotHandler.h"

namespace keryx {

class ProducerImpl {
 public:
   ProducerImpl(keryx_memory_resource &,
                SnapshotHandlerPtr &&, 
                TopicImpl const &,
                keryx_vec<EventPtr> const &initial_snapshot);
   ~ProducerImpl();

   void init(keryx_vec<ConsumerImplPtr> const &consumers);

   void maybe_add(ConsumerImpl &);
   void maybe_remove(ConsumerImpl &);
   void publish(EventPtr &&);

 private:
   bool is_match(StreamFilterImpl const &);
   struct PImpl;
   std::shared_ptr<PImpl> me;
};

} // namespace keryx
