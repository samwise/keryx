#include "ProducerImpl.h"
#include "ConsumerImpl.h"
#include "ProducerType.h"
#include "SnapshotHandler.h"
#include "Topic.h"

#include <atomic>

namespace keryx {

std::atomic<uint64_t> current_producer_id(0);

struct ProducerImpl::PImpl {
   ProducerID id;
   Topic topic;
   std::unique_ptr<SnapshotHandler> snapshot_handler;
   keryx_pmr_small_vector<ConsumerImpl *, 4> consumers;
   NotificationID last_msg_id;
   ProducerTypeDescriptor const &type;
};

ProducerImpl::ProducerImpl() : me() {}

void ProducerImpl::init(
    ProducerTypeDescriptor const &type, Topic const &topic,
    std::vector<EventPtr> const &initial_snapshot,
    std::vector<std::unique_ptr<ConsumerImpl>> const &consumers) {
   me.reset(new PImpl{++current_producer_id,
                      topic,
                      make_snapshot_handler(type.snapshot_policy()),
                      {},
                      0,
                      type});
   for (auto ev : initial_snapshot)
      me->snapshot_handler->add_new_event(type.hash_event(*ev), ev);
   for (auto &c : consumers)
      maybe_add(*c);
}

ProducerImpl::~ProducerImpl() {
   Notification m = {NotificationKind::STOP_PRODUCER, {}, me->id, me->topic};
   for (auto &c : me->consumers)
      c->notify(m);
}

void ProducerImpl::maybe_add(ConsumerImpl &c) {
   if (is_match(c.filter)) {
      me->consumers.push_back(&c);

      Notification m = {
          NotificationKind::START_PRODUCER, {}, me->id, me->topic};
      c.notify(m);
   }
}

void ProducerImpl::maybe_remove(ConsumerImpl &c) {
   auto it = std::find_if(me->consumers.begin(), me->consumers.end(),
                          [&c](auto cptr) { return cptr == &c; });
   if (it != me->consumers.end())
      me->consumers.erase(it);
}

void ProducerImpl::publish(EventPtr ev) {
   auto m = Notification{NotificationKind::EVENT, {ev}, me->id, me->topic};
   me->snapshot_handler->add_new_event(me->type.hash_event(*ev), ev);
   for (auto &c : me->consumers)
      c->notify(m);
}

bool ProducerImpl::is_match(ProducerFilter const &f) {
   return f.producer_type_id == me->type.id() &&
          f.is_match(me->topic.producer_name());
}

} // namespace keryx
