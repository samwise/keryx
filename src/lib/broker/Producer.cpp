#include "Producer.h"
#include "SnapshotHandler.h"
#include "IProducerTypeDescriptor.h"
#include "Consumer.h"

#include <atomic>

namespace keryx {

std::atomic<uint64_t> current_producer_id(0);

struct Producer::PImpl {
   std::vector<Consumer *> consumers;
   Topic topic;
   IProducerTypeDescriptor &descriptor;
   ProducerID id;
   MessageID last_msg_id;
   std::unique_ptr<SnapshotHandler> snapshot_handler;
};

Producer::Producer(Topic const &topic, IProducerTypeDescriptor &descriptor,
                   std::vector<EventPtr> const &initial_snapshot,
                   std::vector<std::unique_ptr<Consumer>> const &consumers)
    : me(new PImpl{{},
                   topic,
                   descriptor,
                   ++current_producer_id,
                   0,
                   make_snapshot_handler(descriptor.snapshot_policy())}) {
   for (auto ev : initial_snapshot)
      me->snapshot_handler->add_new_event(descriptor.hash_event(*ev), ev);
   for (auto &c : consumers)
      maybe_add(*c);
}

Producer::~Producer() {
   Message m = {MessageType::STOP_PRODUCER, {},        me->id,
                ++me->last_msg_id,          me->topic, me->descriptor};
   for (auto &c : me->consumers)
      c->on_message(m);
}

void Producer::maybe_add(Consumer &c) {
   if (is_match(c.filter)) {
      me->consumers.push_back(&c);
      Message m = {MessageType::START_PRODUCER,
                   me->snapshot_handler->get_snapshot(),
                   me->id,
                   ++me->last_msg_id,
                   me->topic,
                   me->descriptor};
      c.on_message(m);
   }
}

void Producer::maybe_remove(Consumer &c) {
   auto it = std::find_if(me->consumers.begin(), me->consumers.end(),
                          [&c](auto cptr) { return cptr == &c; });
   if (it != me->consumers.end())
      me->consumers.erase(it);
}

void Producer::send_event(EventPtr const &ev) {
   auto m = Message{MessageType::EVENT, {ev},      me->id,
                    ++me->last_msg_id,  me->topic, me->descriptor};
   for (auto const &ev : m.events)
      me->snapshot_handler->add_new_event(me->descriptor.hash_event(*ev), ev);
   for (auto &c : me->consumers)
      c->on_message(m);
}

bool Producer::is_match(ProducerFilter const &f) {
   return f.producer_type_id == me->descriptor.type_id() &&
          f.is_match(me->topic);
}

} // namespace keryx
