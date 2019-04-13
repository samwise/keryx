#pragma once
#include "broker_common.h"

namespace keryx {
class ConsumerImpl {
 public:
   ConsumerImpl(StreamFilter const &f, NotificationHandler const &n)
       : filter(f), notify(n) {}
   StreamFilter filter;
   NotificationHandler notify;
};

} // namespace keryx
