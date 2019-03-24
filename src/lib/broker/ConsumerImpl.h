#pragma once
#include "broker_common.h"

namespace keryx {
class ConsumerImpl {
 public:
   ProducerFilter filter;
   NotificationHandler notify;
};

}
