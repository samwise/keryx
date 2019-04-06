#pragma once
#include "broker_common.h"

namespace keryx {

class ProducerImpl {
 public:
   ProducerImpl(keryx_memory_resource &alloc,
                StreamDescriptor const &, Topic const &,
                std::vector<EventPtr> const &initial_snapshot);

   void init(keryx_vec<ConsumerImplPtr> const &consumers);

   ~ProducerImpl();

   void maybe_add(ConsumerImpl &);
   void maybe_remove(ConsumerImpl &);
   void publish(EventPtr);

 private:
   bool is_match(StreamFilter const &);
   struct PImpl;
   std::shared_ptr<PImpl> me;
};

} // namespace keryx
