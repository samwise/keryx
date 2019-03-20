#pragma once
#include "broker_common.h"

namespace keryx {

class ProducerTypeRegistry {
 public:
   virtual ProducerType const &get(ProducerTypeID const &) = 0;
};

}
