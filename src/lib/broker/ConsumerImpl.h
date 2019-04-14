#pragma once
#include "broker_common.h"

namespace keryx {
class ConsumerImpl {
 public:
   ConsumerImpl(StreamFilterImpl const &f, NotificationHandlerImpl const &n)
       : filter(f), notify(n) {}
   StreamFilterImpl filter;
   NotificationHandlerImpl notify;
};

} // namespace keryx
