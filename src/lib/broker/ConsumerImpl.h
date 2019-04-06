#pragma once
#include "broker_common.h"

namespace keryx {
class ConsumerImpl {
 public:
   StreamFilter filter;
   NotificationHandler notify;
};

}
