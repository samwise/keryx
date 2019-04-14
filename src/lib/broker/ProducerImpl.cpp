#include "ProducerImpl.h"
#include "ConsumerImpl.h"
#include "SnapshotHandler.h"
#include "TopicImpl.h"

#include <atomic>

namespace keryx {

std::atomic<uint64_t> current_stream_id;

struct ProducerImpl::PImpl {
   PImpl(keryx_memory_resource &alloc,
         SnapshotHandlerPtr &&handler,
         TopicImpl const &topic)
       : id(++current_stream_id),
         topic(topic),
         snapshot_handler(std::move(handler)),
         consumers(keryx_pmr<ConsumerImpl *>{&alloc}),
         alloc(alloc)
   {}

   StreamID id;
   TopicImpl topic;
   SnapshotHandlerPtr snapshot_handler;
   keryx_vec<ConsumerImpl *> consumers;
   keryx_memory_resource &alloc;
};

ProducerImpl::ProducerImpl(keryx_memory_resource &alloc,
                           SnapshotHandlerPtr &&handler,
                           TopicImpl const &topic,
                           keryx_vec<EventPtr> const &initial_snapshot)
    : me(std::allocate_shared<PImpl>(keryx_pmr<PImpl>{&alloc},
                                     alloc,
                                     std::move(handler),
                                     topic)) {
   me->snapshot_handler->init(initial_snapshot);
}

void ProducerImpl::init(keryx_vec<ConsumerImplPtr> const &consumers) {
   for (auto &c : consumers)
      maybe_add(*c);
}

ProducerImpl::~ProducerImpl() {
   NotificationImpl m = {NotificationKind::STOP_PRODUCER, {}, me->id, me->topic};
   for (auto &c : me->consumers)
      c->notify(m);
}

void ProducerImpl::maybe_add(ConsumerImpl &c) {
   if (is_match(c.filter)) {
      me->consumers.push_back(&c);
      c.notify({NotificationKind::START_PRODUCER, {}, me->id, me->topic});
   }
}

void ProducerImpl::maybe_remove(ConsumerImpl &c) {
   auto it = std::find_if(me->consumers.begin(), me->consumers.end(),
                          [&c](auto cptr) { return cptr == &c; });
   if (it != me->consumers.end())
      me->consumers.erase(it);
}

void ProducerImpl::publish(EventPtr &&ev) {
   auto m = NotificationImpl{NotificationKind::EVENT, {ev.get()}, me->id, me->topic};
   for (auto &c : me->consumers)
      c->notify(m);
   me->snapshot_handler->add_new_event(std::move(ev));
}

bool ProducerImpl::is_match(StreamFilterImpl const &f) {
   return f.stream_type_id == me->topic.stream_type_id() &&
          f.is_match(me->topic.stream_name());
}

} // namespace keryx
