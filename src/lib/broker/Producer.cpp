#include "Producer.h"
#include "Consumer.h"
#include "ProducerType.h"
#include "SnapshotHandler.h"
#include "Topic.h"

#include <atomic>

namespace keryx {

std::atomic<uint64_t> current_producer_id(0);

struct Producer::PImpl {
   ProducerType const &type;
   Topic topic;
   ProducerID id;
   std::vector<Consumer *> consumers;
   NotificationID last_msg_id;
   std::unique_ptr<SnapshotHandler> snapshot_handler;
};

Producer::Producer(ProducerType const &type, Topic const &topic,
                   std::vector<EventPtr> const &initial_snapshot,
                   std::vector<std::unique_ptr<Consumer>> const &consumers)
    : me(new PImpl{type,
                   topic,
                   ++current_producer_id,
                   {},
                   0,
                   make_snapshot_handler(type.snapshot_policy())}) {
   for (auto ev : initial_snapshot)
      me->snapshot_handler->add_new_event(type.hash_event(*ev), ev);
   for (auto &c : consumers)
      maybe_add(*c);
}

Producer::~Producer() {
   Notification m = {NotificationKind::STOP_PRODUCER, {}, me->id, me->topic};
   for (auto &c : me->consumers)
      c->notify(m);
}

void Producer::maybe_add(Consumer &c) {
   if (is_match(c.filter)) {
      me->consumers.push_back(&c);
      Notification m = {NotificationKind::START_PRODUCER,
                        me->snapshot_handler->get_snapshot(), me->id,
                        me->topic};
      c.notify(m);
   }
}

void Producer::maybe_remove(Consumer &c) {
   auto it = std::find_if(me->consumers.begin(), me->consumers.end(),
                          [&c](auto cptr) { return cptr == &c; });
   if (it != me->consumers.end())
      me->consumers.erase(it);
}

void Producer::publish(EventPtr const &ev) {
   auto m = Notification{NotificationKind::EVENT, {ev}, me->id, me->topic};
   for (auto const &ev : m.events)
      me->snapshot_handler->add_new_event(me->type.hash_event(*ev), ev);
   for (auto &c : me->consumers)
      c->notify(m);
}

bool Producer::is_match(ProducerFilter const &f) {
   return f.producer_type_id == me->type.id() &&
          f.is_match(me->topic.producer_name());
}

} // namespace keryx
