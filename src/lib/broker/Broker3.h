#pragma once

#include <atomic>
#include <functional>
#include <inplace_function.h>
#include <memory>
#include <mpmc_bounded_queue.h>
#include <mutex>
#include <vector>

#include "lib/utils/mp_utils.h"

// Open issues:
// - late join
// - full queue and deadlocks
// - use of neighbour in subscription to avoid forwarding subs that
//   are not needed
// - request/reply

namespace keryx3 {

using SubID = uint32_t;
template <typename E> using Handler = std::function<void(const E &)>;
template <typename B>
using BrokerAction = stdext::inplace_function<void(B &), 128>;

template <class EV, class Broker> struct EventRecord {
   std::vector<std::pair<SubID, Handler<EV>>> local_handlers;
   std::atomic<SubID> next_handler_id = 1;

   template <class C> void sub(SubID sid, C const &c) {
      local_handlers.emplace_back(sid, Handler<EV>(c));
   }

   void unsub(SubID id) {
      auto it = std::find_if(local_handlers.begin(), local_handlers.end(),
                             [id](auto p) { return p.first == id; });
      if (it != local_handlers.end())
         local_handlers.erase(it);
   }
};

template <typename... E> struct Broker {

   using EventRecordTuple = std::tuple<EventRecord<E, Broker>...>;
   template <class EV> EventRecord<EV, Broker> &get() {
      constexpr auto idx = keryx::GetTypeIdxInTuple<EventRecord<EV, Broker>,
                                                    EventRecordTuple>::value;
      return std::get<idx>(event_records);
   }

   EventRecordTuple event_records;
   spdlog::details::mpmc_bounded_queue<BrokerAction<Broker>> notification_queue;
   std::vector<Broker *> neighbours;

   Broker() : event_records(), notification_queue(1024), neighbours() {}

   template <class EV> void publish(EV const &ev) {

      for (auto n : neighbours) {
         BrokerAction<Broker> nt = [ev](Broker &b) {
            auto &handlers = b.template get<EV>().local_handlers;
            for (auto &h : handlers)
               h.second(ev);
         };
         n->enqueue(nt);
      }
   }

   void add_neighbour(std::initializer_list<Broker *> const &brokers) {
      for (auto n : brokers)
         neighbours.push_back(n);
   }

   void do_work() {
      BrokerAction<Broker> nt;
      while (dequeue(nt))
         nt(*this);
   }

   template <class EV, class C> SubID subscribe(C const &c) {
      auto id = get<EV>().next_handler_id++;
      enqueue([c, id](Broker &self) { self.get<EV>().sub(id, c); });
      return id;
   }

   template <class EV> void unsubscribe(SubID id) {
      enqueue([id](Broker &self) { self.get<EV>().unsub(id); });
   }

   void enqueue(BrokerAction<Broker> const &n) {

      notification_queue.enqueue(n);
   }

   bool dequeue(BrokerAction<Broker> &n) {
      return notification_queue.dequeue(n);
   }
};

} // namespace keryx3
