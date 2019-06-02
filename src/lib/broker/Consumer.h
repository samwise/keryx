#pragma once
#include "OldBroker.h"
#include "broker_common.h"

namespace keryx {

class Broker;

template <class Stream> class Notification {
 public:
   using EventType = typename Stream::EventType;
   NotificationKind kind;
   keryx_small_vec<EventType const *, 1> events;
   StreamID stream_id;
   StreamName const &name;
};

template <class Stream> class Consumer {
 public:
   using EventType = typename Stream::EventType;

   template <class Filter, class Handler>
   Consumer(OldBroker &b, Filter const &f, Handler const &h)
       : broker(b), consumer(broker.make_consumer({typeid(Stream), f},
                                                  make_untyped_handler(h))) {}

   template <class Handler>
   Consumer(OldBroker &b, StreamName const &name, Handler const &h)
       : broker(b),
         consumer(broker.make_consumer(
             {typeid(Stream), [name](auto const &n) { return name == n; }},
             make_untyped_handler(h))) {}

   template <class Handler>
   Consumer(OldBroker &b, Handler const &h)
       : broker(b), consumer(broker.make_consumer(
                        {typeid(Stream), [](auto const &) { return true; }},
                        make_untyped_handler(h))) {}

   ~Consumer() { broker.destroy_consumer(consumer); }

 private:
   template <class H> NotificationHandlerImpl make_untyped_handler(H const &h) {
      return [h](auto const &untyped_notification) {
         Notification<Stream> typed_notification{
             untyped_notification.kind,
             {},
             untyped_notification.stream_id,
             untyped_notification.topic.stream_name()};
         for (auto ev : untyped_notification.events)
            typed_notification.events.push_back(
                dynamic_cast<EventType const *>(ev));
         h(typed_notification);
      };
   }

   OldBroker &broker;
   ConsumerImpl &consumer;
}; // namespace keryx

} // namespace keryx
