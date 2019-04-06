#include "Consumer.h"
#include "Broker.h"
#include "ConsumerImpl.h"

namespace keryx {

struct Consumer::PImpl {
   NotificationHandler on_notify;
   Broker *broker;
   ConsumerImpl *consumer;
};

Consumer::Consumer(Broker &b, StreamFilter const &f,
                   NotificationHandler const &h)
    : me(new PImpl{}) {

   me->on_notify = [this](auto const &n) { me->on_notify(n); };
   me->broker = &b;
   me->consumer = &(me->broker->make_consumer(f, h));
}

Consumer::~Consumer() {
   me->on_notify = [](auto const &) {};
   me->broker->destroy_consumer(*me->consumer);
}
} // namespace keryx
