#include "QueuedBroker.h"
#include "Broker.h"
#include "ConsumerImpl.h"
#include "ProducerImpl.h"
#include "Topic.h"

#include <queue>

namespace keryx {

class ProducerTypeRegistry {
 public:
   virtual ProducerTypeDescriptor const &get(ProducerTypeID const &) = 0;
};

using Action = std::function<void(Broker &)>;

struct QueuedBroker::PImpl {
   std::unique_ptr<Broker> broker;
   std::queue<Action> pending_actions;
};

QueuedBroker::QueuedBroker(ProducerTypeRegistry &r)
    : me(new PImpl{std::make_unique<Broker>(r), {}}) {}

QueuedBroker::~QueuedBroker() {}

ProducerImpl &QueuedBroker::add_producer(ProducerImpl &p, Topic const &topic,
                                         std::vector<EventPtr> const &initial) {
   me->pending_actions.push(
       [&p, topic, initial](Broker &b) { b.add_producer(p, topic, initial); });
   return p;
}

void QueuedBroker::publish(ProducerImpl &p, EventPtr const &ev) {

   me->pending_actions.push([&p,ev](Broker &b) {
         b.publish(p, ev);
      });
}

void QueuedBroker::destroy_producer(ProducerImpl &p) {
   me->pending_actions.push([&p](Broker &b) { b.destroy_producer(p); });
}

ConsumerImpl &QueuedBroker::add_consumer(ConsumerImpl &c,
                                         ProducerFilter const &f,
                                         NotificationHandler const &h) {
   me->pending_actions.push([&c, f, h](Broker &b) { b.add_consumer(c, f, h); });
   return c;
}

void QueuedBroker::destroy_consumer(ConsumerImpl &c) {
   me->pending_actions.push([&c](Broker &b) { b.destroy_consumer(c); });
}

void QueuedBroker::do_work()
{
   while (me->pending_actions.size()) {
      me->pending_actions.front()(*me->broker);
      me->pending_actions.pop();
   }
}

} // namespace keryx
