#include "Consumer.h"
#include "ConsumerImpl.h"
#include "QueuedBroker.h"

namespace keryx {

struct Consumer::PImpl {
   NotificationHandler on_notify;
   QueuedBroker *broker;
   ConsumerImpl *consumer;
};

Consumer::Consumer(QueuedBroker &b, ProducerFilter const &f,
                   NotificationHandler const &h)
   : me(new PImpl{}) {

   me->on_notify = [this](auto const &n) { me->on_notify(n); };
   me->broker = &b;
   me->consumer = &(me->broker->add_consumer(*new ConsumerImpl(), f, h));
}

Consumer::~Consumer() {
   me->on_notify = [](auto const &) {};
   me->broker->destroy_consumer(*me->consumer);
}
} // namespace keryx
