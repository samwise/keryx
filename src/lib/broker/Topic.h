#pragma once

#include "broker_common.h"

namespace keryx {
class Topic {
 public:
   Topic(ProducerTypeID const &type_id, ProducerName const &name)
       : _producer_type_id(type_id), _producer_name(name) {}

   ProducerTypeID const &producer_type_id() const { return _producer_type_id; }
   ProducerName const &producer_name() { return _producer_name; }
   ~Topic() {} // necessary because of clang bug
 private:
   ProducerTypeID _producer_type_id;
   ProducerName _producer_name;
};
} // namespace keryx
