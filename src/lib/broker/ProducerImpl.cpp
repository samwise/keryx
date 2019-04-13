#include "ProducerImpl.h"
#include "ConsumerImpl.h"
#include "SnapshotHandler.h"
#include "StreamDescriptor.h"
#include "Topic.h"

#include <atomic>

namespace keryx {

std::atomic<uint64_t> current_producer_id;

struct ProducerImpl::PImpl {
   PImpl(keryx_memory_resource &alloc, Topic const &topic,
         SnapshotHandlerPtr &&handler, StreamDescriptor const &desc)
       : id(++current_producer_id), topic(topic),
         snapshot_handler(std::move(handler)),
         consumers(keryx_pmr<ConsumerImpl *>{&alloc}), descriptor(desc),
         alloc(alloc)
   {}

   StreamID id;
   Topic topic;
   SnapshotHandlerPtr snapshot_handler;
   keryx_vec<ConsumerImpl *> consumers;
   NotificationID last_msg_id;
   StreamDescriptor const &descriptor;
   keryx_memory_resource &alloc;
};

ProducerImpl::ProducerImpl(keryx_memory_resource &alloc,
                           StreamDescriptor const &desc, Topic const &topic,
                           std::vector<Event const *> const &initial_snapshot)
    : me(std::allocate_shared<PImpl>(keryx_pmr<PImpl>{&alloc}, alloc, topic,
                                     desc.make_snapshot_handler(alloc), desc)) {

   for (auto ev : initial_snapshot) {
      auto ev_clone = desc.clone_event(*ev,alloc);
      me->snapshot_handler->add_new_event(std::move(ev_clone));
   }
}

EventPtr ProducerImpl::clone_event(Event const &ev){
   return me->descriptor.clone_event(ev,me->alloc);
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

void ProducerImpl::publish(EventPtr &&ev) {
   auto m = Notification{NotificationKind::EVENT, {ev.get()}, me->id, me->topic};
   for (auto &c : me->consumers)
      c->notify(m);
   me->snapshot_handler->add_new_event(std::move(ev));
}

bool ProducerImpl::is_match(StreamFilter const &f) {
   return f.stream_type == me->descriptor.stream_type() &&
          f.is_match(me->topic.producer_name());
}

} // namespace keryx
