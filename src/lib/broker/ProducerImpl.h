#pragma once
#include "broker_common.h"

namespace keryx {

class ProducerImpl {
 public:
   ProducerImpl();
   void init(ProducerTypeDescriptor const &, Topic const &,
             std::vector<EventPtr> const &initial_snapshot,
             std::vector<std::unique_ptr<ConsumerImpl>> const &consumers);

   ~ProducerImpl();

   void maybe_add(ConsumerImpl &);
   void maybe_remove(ConsumerImpl &);
   void publish(EventPtr);

 private:
   bool is_match(ProducerFilter const &);
   struct PImpl;
   std::unique_ptr<PImpl> me;
};

} // namespace keryx
