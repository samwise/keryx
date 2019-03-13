#pragma once
#include "../utils/allocators.h"
#include "broker_common.h"

namespace keryx {

class IProducerTypeDescriptor;

class Broker {
 public:
   Broker();
   ~Broker();

   Producer &make_producer(Topic const &, IProducerTypeDescriptor &,
                           std::vector<EventPtr> const & initial = {});
   void send_event(Producer &, EventPtr const &);
   void destroy_producer(Producer &);

   Consumer &make_consumer(ProducerFilter const &, MessageHandler const &);
   void destroy_consumer(Consumer &);

 private:
   struct PImpl;
   std::unique_ptr<PImpl> me;
};

} // namespace keryx
