#include "Producer.h"
#include "QueuedBroker.h"
#include "ProducerImpl.h"

namespace keryx {

struct Producer::PImpl {
   QueuedBroker *broker;
   ProducerImpl *producer;
};

Producer::Producer(QueuedBroker &b, Topic const &topic,
                   std::vector<EventPtr> const &initial_snapshot)
   : me(new PImpl{}) {
   me->broker = &b;
   me->producer =
      &(me->broker->add_producer(*new ProducerImpl(), topic, initial_snapshot));
}

Producer::~Producer() { me->broker->destroy_producer(*me->producer); }

void Producer::publish(const Event &event) {
   me->broker->publish(*me->producer, &event);
}

} // namespace keryx
