#pragma once
#include "broker_common.h"

namespace keryx {
class Consumer {
 public:
   ProducerFilter filter;
   NotificationHandler notify;
};

}
