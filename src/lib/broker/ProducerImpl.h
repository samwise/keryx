#pragma once
#include "broker_common.h"

namespace keryx {

class ProducerImpl {
 public:
   ProducerImpl(keryx_memory_resource &, StreamDescriptor const &,
                Topic const &,
                std::vector<Event const *> const &initial_snapshot);

   void init(keryx_vec<ConsumerImplPtr> const &consumers);

   ~ProducerImpl();

   EventPtr clone_event(Event const &);
   void maybe_add(ConsumerImpl &);
   void maybe_remove(ConsumerImpl &);
   void publish(EventPtr &&);

 private:
   bool is_match(StreamFilter const &);
   struct PImpl;
   std::shared_ptr<PImpl> me;
};

} // namespace keryx
