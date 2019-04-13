#include "Producer.h"
#include "Broker.h"
#include "ProducerImpl.h"

namespace keryx {

struct Producer::PImpl {
   Broker *broker;
   ProducerImpl *producer;
};

Producer::Producer(Broker &b, Topic const &topic,
                   std::vector<Event const *> const &initial_snapshot)
    : me(new PImpl{}) {
   me->broker = &b;
   me->producer = &(me->broker->make_producer(topic, initial_snapshot));
}

Producer::~Producer() { me->broker->destroy_producer(*me->producer); }

void Producer::publish(const Event &event) {
   me->broker->publish(*me->producer, event);
}

} // namespace keryx
