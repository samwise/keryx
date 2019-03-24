#pragma once
#include "broker_common.h"

namespace keryx {

class ProducerTypeRegistry {
 public:
   virtual ProducerTypeDescriptor const &get(ProducerTypeID const &) = 0;
};

}
