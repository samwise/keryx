#pragma once

#include <atomic>
#include <functional>
#include <inplace_function.h>
#include <memory>
#include <moodycamel/readerwriterqueue.h>
#include <mpmc_bounded_queue.h>
#include <mutex>
#include <vector>

#include "lib/utils/mp_utils.h"

// Open issues:
// - late join
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
   std::vector<Queue *> remote_handlers;
   std::atomic<SubID> next_handler_id = 1;
};

template <typename... E> struct Broker {

   using EventRecordTuple = std::tuple<EventRecord<E, Broker>...>;
   template <class EV> EventRecord<EV, Broker> &get() {
      constexpr auto idx = keryx::GetTypeIdxInTuple<EventRecord<EV, Broker>,
                                                    EventRecordTuple>::value;
      return std::get<idx>(event_records);
   }

   EventRecordTuple event_records;

   using Queue = moodycamel::ReaderWriterQueue<BrokerAction<Broker>>;
   std::vector<std::pair<Broker *, Queue *>> out;
   std::vector<std::unique_ptr<Queue>> in;
   Queue *self_queue;

   Broker() : event_records(), out(), in(), self_queue() {}

   template <class EV> void publish(EV const &ev) {

      BrokerAction<Broker> nt = [ev](Broker &b) {
         auto &handlers = b.template get<EV>().local_handlers;
         for (auto &h : handlers)
            h.second(ev);
      };
      self_queue->enqueue(nt);
      for (auto const &q : get<EV>().remote_handlers)
         q->enqueue(nt);
   }

   void add_neighbour(std::initializer_list<Broker *> const &brokers) {
      for (auto b : brokers)
         if (b != this) {
            auto &q = b->in.emplace_back(new Queue());
            out.push_back(std::make_pair(&b, &q));
         }
      auto &q = in.emplace_back(new Queue());
      self_queue = q.get();
   }

   void do_work() {
      BrokerAction<Broker> nt;
      for (auto &q : in)
         while (q->try_dequeue(nt))
            nt(*this);
   }

   template <class EV, class C> SubID subscribe(C const &c) {
      auto id = get<EV>().next_handler_id++;

      self_queue->enqueue([c, id](Broker &self) {
         // local sub
         auto &er = self.get<EV>();
         er.local_handlers.push_back(std::make_pair(id, c));

         // first sub, forward to neighbours
         if (er.local_landers.size() == 1) {
            for (auto &p : out) {
               auto q = p.second;
               q->enqueue([self](Broker &b) {
                  auto it = std::find_if(
                      b.out.begin(), b.out.end(),
                      [self](auto &nb) { return nb.first == self; });
                  auto src_q = it->second;
                  auto &er = b.get<EV>();
                  er.remote_handlers.push_back(src_q);
               });
            }
         }
      });
      return id;
   }

   template <class EV> void unsubscribe(SubID id) {
      self_queue->enqueue([id](Broker &self) {
         auto &er = self.get<EV>();
         auto it = std::find_if(er.local_handlers.begin(), er.local_handlers.end(),
                                [id] (auto &p) {return p.first == id;}); 
         if (it != er.local_handlers.end())
            er.local_handlers.erase(it);

         // if last sub fwd unsub
         if (er.local_handlers.empty()) {
            for (auto &p : out) {
               auto q = p.second;
               q->enqueue([self](Broker &b) {
                  auto it = std::find_if(
                     b.out.begin(), b.out.end(),
                     [self](auto &nb) { return nb.first == self; });
                  auto src_q = it->second;
                  auto &er = b.get<EV>();
                  auto it2 = st

                     
                  er.remote_handlers.push_back(it->second);
                  });
            }
         }
      });
   }
};

} // namespace keryx3
