#include "Consumer.h"
#include "Broker.h"

namespace keryx {

struct Consumer::PImpl {
   NotificationHandler on_notify;
   Broker &broker;
   ConsumerImpl &consumer;
};

Consumer::Consumer(Broker &b, ProducerFilter const &f,
                   NotificationHandler const &h)
    : me(new PImpl{h, b, b.make_consumer(f, [this](auto const &n) {
                      me->on_notify(n);
                   })}) {}

Consumer::~Consumer() {
   me->on_notify = [](auto const &) {};
   me->broker.destroy_consumer(me->consumer);
}
} // namespace keryx
