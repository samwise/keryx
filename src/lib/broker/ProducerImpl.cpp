#include "ProducerImpl.h"
#include "ConsumerImpl.h"
#include "StreamDescriptor.h"
#include "SnapshotHandler.h"
#include "Topic.h"

#include <atomic>

namespace keryx {

std::atomic<uint64_t> current_producer_id;

struct ProducerImpl::PImpl {
   PImpl(keryx_memory_resource &alloc, Topic const &topic,
         std::shared_ptr<SnapshotHandler> const &handler,
         StreamDescriptor const &type)
       : id(++current_producer_id), topic(topic), snapshot_handler(handler),
         consumers(keryx_pmr<ConsumerImpl *>{&alloc}), type(type) {}

   StreamID id;
   Topic topic;
   std::shared_ptr<SnapshotHandler> snapshot_handler;
   keryx_vec<ConsumerImpl *> consumers;
   NotificationID last_msg_id;
   StreamDescriptor const &type;
};

ProducerImpl::ProducerImpl(keryx_memory_resource &alloc,
                           StreamDescriptor const &desc,
                           Topic const &topic,
                           std::vector<EventPtr> const &initial_snapshot)
    : me(std::allocate_shared<PImpl>(
            keryx_pmr<PImpl>{&alloc}, alloc,topic,
          make_snapshot_handler(desc.snapshot_policy()), desc)) {

   for (auto ev : initial_snapshot)
      me->snapshot_handler->add_new_event(desc.hash_event(*ev), ev);
}

void ProducerImpl::init(keryx_vec<ConsumerImplPtr> const &consumers) {
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

bool ProducerImpl::is_match(StreamFilter const &f) {
   return f.stream_type_id == me->type.id() &&
          f.is_match(me->topic.producer_name());
}

} // namespace keryx
