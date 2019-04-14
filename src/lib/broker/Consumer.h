#pragma once
#include "Broker.h"
#include "broker_common.h"

namespace keryx {

class Broker;

template <class Stream> class Notification {
 public:
   using EventType = typename Stream::EventType;
   NotificationKind kind;
   keryx_small_vec<EventType const*, 1> events;
   StreamID stream_id;
   StreamName const &name;
};

template <class Stream> class Consumer {
 public:
   using EventType = typename Stream::EventType;

   template <class Filter, class Handler>
   Consumer(Broker &b, Filter const &f, Handler const &h)
       : broker(b),
         consumer(broker.make_consumer(
             {typeid(Stream), f}, [h](auto const &untyped_notification) {
                Notification<Stream> typed_notification{
                    untyped_notification.kind,
                    {},
                    untyped_notification.stream_id,
                    untyped_notification.topic.stream_name()};
                for (auto ev : untyped_notification.events)
                   typed_notification.events.push_back(
                       dynamic_cast<EventType const*>(ev));
                h(typed_notification);
             })) {}

   ~Consumer() { broker.destroy_consumer(consumer); }

 private:
   Broker &broker;
   ConsumerImpl &consumer;
}; // namespace keryx

} // namespace keryx
