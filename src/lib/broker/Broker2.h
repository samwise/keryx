#pragma once

#include <functional>
#include <memory>
#include <mpmc_bounded_queue.h>
#include <vector>
#include <atomic>
#include <mutex>
#include <vector>

#include "lib/utils/mp_utils.h"

namespace keryx2 {

using SubID = uint32_t;
class SpinLock
{
public:
   void lock() {
      while (_mutex.test_and_set(std::memory_order_acquire));
   }
   void unlock() {
      _mutex.clear(std::memory_order_release);
   }

private:
   std::atomic_flag _mutex = ATOMIC_FLAG_INIT;
};

template <typename E> using Handler = std::function<void(const E &)>;

template <class Broker>
struct NotificationBase {
   virtual void notify(Broker &) = 0;
   virtual ~NotificationBase() {}
};

template <class EV,class Broker>
struct Notification : public NotificationBase<Broker>{
   EV event;
   std::atomic<int> rc;
   Broker *sender;

   void notify(Broker & receiver) override {
      auto &handlers = receiver.template get<EV>().handlers;
      for(auto h: handlers)
         h.second(event);
      auto remaining = rc.fetch_sub(1);
      if(remaining==1) 
         sender->template get<EV>().free(*this);
   }
};

template <class EV, class Broker>
struct EventRecord {
   using Not = Notification<EV,Broker>;
   SpinLock pool_lock;
   std::vector<Not*> free_list;
   std::vector<std::pair<SubID, Handler<EV>>> handlers;
   std::vector<std::unique_ptr<Not>> pool;
   SubID next_handler_id = 1;

   EventRecord() {
      std::vector<Not*> allocated;
      for (int i = 0; i < 100; i++)
         allocated.push_back(&alloc());
      for (auto p : allocated)
         free(*p);
   }

   Not& alloc() {
      std::lock_guard<SpinLock> g(pool_lock);
      
      if (free_list.empty()) {
         auto const &n_ptr = pool.emplace_back(new Not());
         return *n_ptr;
      } else {
         auto n = free_list.back();
         free_list.pop_back();
         return *n;
      }
   }

   void free(Not &n) {
      std::lock_guard<SpinLock> g(pool_lock);
      free_list.push_back(&n);
   }

   template <class C> SubID sub(C const &c) {
      handlers.emplace_back(next_handler_id, Handler<EV>(c));
      return next_handler_id++;
   }

   void unsub(SubID id) {
      auto it = std::find_if(handlers.begin(), handlers.end(),
                             [id](auto p) { return p.first == id; });
      if (it != handlers.end())
         handlers.erase(it);
   }
};

template <typename... E> struct Broker {

   using EventRecordTuple = std::tuple<EventRecord<E,Broker>...>;
   template <class EV> EventRecord<EV,Broker> &get() {
      constexpr auto idx =
         keryx::GetTypeIdxInTuple<EventRecord<EV,Broker>, EventRecordTuple>::value;
      return std::get<idx>(event_records);
   }

   EventRecordTuple event_records;
   spdlog::details::mpmc_bounded_queue<NotificationBase<Broker>*> notification_queue;
   std::vector<Broker *> neighbours;

   Broker() : event_records(), notification_queue(1024), neighbours() {}
   
   template <class EV, typename... Args> void publish(Args &&... args) {
      auto &nt = get<EV>().alloc();
      nt.event.update(std::forward<Args>(args)...);
      nt.sender = this;
      nt.rc = neighbours.size();
      
      for (auto n : neighbours)
         n->enqueue(&nt);
   }

   void add_neighbour(std::initializer_list<Broker *> const &brokers) {
      for (auto n : brokers)
         neighbours.push_back(n);
   }

   void do_work() {
      NotificationBase<Broker> *nt;
      while (dequeue(nt))
         nt->notify(*this);
   }

   bool enqueue(NotificationBase<Broker> *n) { return notification_queue.enqueue(n); }
   bool dequeue(NotificationBase<Broker> *&n) { return notification_queue.dequeue(n); }
   template <class EV, class C> SubID subscribe(C const &c) {
      return get<EV>().sub(c);
   }

   template <class EV> void unsubscribe(SubID id) {
      return get<EV>().unsub(id);
   }
};

}
