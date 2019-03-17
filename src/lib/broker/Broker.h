#pragma once
#include "broker_common.h"

namespace keryx {

class ProducerTypeRegistry;

class Broker {
 public:
   Broker(ProducerTypeRegistry &);
   ~Broker();

   Producer &make_producer(Topic const &, std::vector<EventPtr> const & initial = {});
   void publish(Producer &, EventPtr const &);
   void destroy_producer(Producer &);

   Consumer &make_consumer(ProducerFilter const &, NotificationHandler const &);
   void destroy_consumer(Consumer &);

 private:
   struct PImpl;
   std::unique_ptr<PImpl> me;
};

} // namespace keryx
