#pragma once

#include "broker_common.h"

namespace keryx {
class Topic {
 public:
   ProducerTypeID const &producer_type_id() const {return _producer_type_id;}
   ProducerName const &producer_name() {return _producer_name;} 
   ~Topic() {}  // necessary because of clang bug
private:
   ProducerTypeID _producer_type_id;
   ProducerName _producer_name;
};
} // namespace keryx
