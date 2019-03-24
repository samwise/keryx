#include "Producer.h"
#include "Broker.h"

namespace keryx {

class ProducerImpl;

struct Producer::PImpl {
   Broker &broker;
   ProducerImpl &producer;
};

Producer::Producer(Broker &b,
                   Topic const &topic,
                   std::vector<EventPtr> const &initial_snapshot)
   : me(new PImpl{b,b.make_producer(topic,initial_snapshot)})
{}

Producer::~Producer() {
   me->broker.destroy_producer(me->producer);
}

void Producer::publish(const Event&event) {
   me->broker.publish(me->producer, &event);
}

}
