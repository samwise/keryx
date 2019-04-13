#pragma once
#include "broker_common.h"

namespace keryx {

class StreamDescriptorRegistry {
 public:
   virtual StreamDescriptor const &get(StreamType const &) = 0;
};

}
