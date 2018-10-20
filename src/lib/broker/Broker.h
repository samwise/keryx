#pragma once

#include <functional>
#include <memory>
#include <mpmc_bounded_queue.h>
#include <vector>
#include <inplace_function.h>

#include "lib/utils/mp_utils.h"

namespace keryx {

template <typename E> using Handler = std::function<void(const E &)>;

using BrokerAction = stdext::inplace_function<void(),32>;
using SubID = uint32_t;
struct EventBase {
   uint32_t in_flight = 0;
};

template <typename E> struct EventRecord {
   std::vector<E *> free_list;
   std::vector<std::pair<SubID, Handler<E>>> handlers;
   std::vector<std::unique_ptr<E>> pool;
   SubID next_handler_id = 1;

   EventRecord() {
      std::vector<E*> allocated;
      for (int i = 0; i < 100; i++)
         allocated.push_back(&alloc());
      for (auto p : allocated)
         free(*p);
   }

   E &alloc() {
      if (free_list.empty()) {
         auto const &ev_ptr = pool.emplace_back(new E());
         return *ev_ptr;
      } else {
         auto ev = free_list.back();
         free_list.pop_back();
         return *ev;
      }
   }

   void free(E &e) { free_list.push_back(&e); }

   template <class C> SubID sub(C const &c) {
      handlers.emplace_back(next_handler_id, Handler<E>(c));
      return next_handler_id++;
   }

   void unsub(SubID id) {
      auto it = std::find_if(handlers.begin(), handlers.end(),
                             [id](auto p) { return p.first == id; });
      if (it != handlers.end())
         handlers.erase(it);
   }
};

template <typename... E> class Broker {
 private:
   using EventRecordTuple = std::tuple<EventRecord<E>...>;
   template <class EV> EventRecord<EV> &get() {
      constexpr auto idx =
          GetTypeIdxInTuple<EventRecord<EV>, EventRecordTuple>::value;
      return std::get<idx>(event_records);
   }

   EventRecordTuple event_records;
   spdlog::details::mpmc_bounded_queue<BrokerAction> queue;
   std::vector<Broker *> neighbours;

 public:
   Broker() : event_records(), queue(1024), neighbours() {}
   template <class EV, typename... Args> void publish(Args &&... args) {
      auto &ev = get<EV>().alloc();
      ev.update(std::forward<Args>(args)...);
      ev.in_flight = neighbours.size();
      
      for (auto n : neighbours) {
         auto sender = this;
         BrokerAction a = [n, &ev,sender]() {
            auto &handlers = n->template get<EV>().handlers;
            for (auto &h : handlers)
               h.second(ev);
            BrokerAction reply = [&ev, sender]() {
               ev.in_flight--;
               if (ev.in_flight == 0)
                  sender->template get<EV>().free(ev);
            };
            sender->enqueue(reply);
         };
         n->enqueue(a);
      }
   }

   void add_neighbour(std::initializer_list<Broker *> const &brokers) {
      for (auto n : brokers)
         if (n != this)
            neighbours.push_back(n);
   }

   void do_work() {
      BrokerAction a;
      while (dequeue(a))
         a();
   }

   void enqueue(BrokerAction const &a) { queue.enqueue(a); }

   bool dequeue(BrokerAction &a) { return queue.dequeue(a); }

   template <class EV, class C> SubID subscribe(C const &c) {
      return get<EV>().sub(c);
   }

   template <class EV> void unsubscribe(SubID id) {
      return get<EV>().unsub(id);
   }
};
} // namespace keryx
