

#include <functional>
#include <memory>
#include <queue>
#include <vector>

namespace keryx {

// meta nonsense
template <int N, typename Target, typename...> struct GetTypeIdxImp {};
template <int N, typename Target> struct GetTypeIdxImp<N, Target> {};
template <int N, typename Target, typename First, typename... Rest>
struct GetTypeIdxImp<N, Target, First, Rest...> {
   static constexpr int value =
       std::conditional<std::is_same<Target, First>::value,
                        std::integral_constant<int, N>,
                        GetTypeIdxImp<N + 1, Target, Rest...>>::type::value;
};
template <typename Target, typename... TypeList> struct GetTypeIdx {
   static constexpr int value = GetTypeIdxImp<0, Target, TypeList...>::value;
};
template <typename Target, class... Types> struct GetTypeIdxInTuple {};
template <typename Target, class... TypeList>
struct GetTypeIdxInTuple<Target, std::tuple<TypeList...>> {
   static constexpr int value = GetTypeIdxImp<0, Target, TypeList...>::value;
};
// meta nonsense

template <typename E> using Handler = std::function<void(const E &)>;

using BrokerAction = std::function<void()>;
using SubID = uint32_t;

template <typename E> struct EventRecord {
   std::vector<E *> free_list;
   std::vector<std::pair<SubID, Handler<E>>> handlers;
   std::vector<std::unique_ptr<E>> pool;
   SubID next_handler_id = 1;

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
   using EventRecordTuple = std::tuple<EventRecord<E>...> ;
   template <class EV> EventRecord<EV> &get() {
      constexpr auto idx = GetTypeIdxInTuple<EventRecord<EV>,EventRecordTuple>::value;
      return std::get<idx>(event_records);
   }

   EventRecordTuple event_records;
   std::queue<BrokerAction> queue;
   std::vector<Broker *> neighbours;

 public:
   template <class EV, typename... Args> void publish(Args &&... args) {
      auto &ev = get<EV>().alloc();
      ev.update(std::forward<Args>(args)...);

      for (auto n : neighbours) {
         auto sender = this;
         BrokerAction a = [n, &ev, sender]() {
            auto &handlers = n->template get<EV>().handlers;
            for (auto &h : handlers)
               h.second(ev);
            BrokerAction reply = [&ev, sender]() {
               ev.in_flight--;
               if (ev.in_flight == 0)
                  sender->template get<EV>().free(ev);
            };
            sender->queue.push(reply);
         };
         n->queue.push(a);
      }
   }

   void do_work() {
      while (!queue.empty()) {
         queue.front()();
         queue.pop();
      }
   }

   template <class EV, class C> SubID subscribe(C const &c) {
      return get<EV>().sub(c);
   }

   template <class EV> void unsubscribe(SubID id) {
      return get<EV>().unsub(id);
   }
};

struct EventBase {
   uint32_t in_flight = 0;
};

struct EVA : public EventBase {void update(){}};
struct EVB : public EventBase {void update(){}};

Broker<EVA,EVB> b1;

void f() {
   using BR = Broker<EVA,EVB> ;
   BR br1;
   BR br2;

   br1.publish<EVA>();
   auto id = br1.subscribe<EVA>([] (auto const &) {});
   br1.unsubscribe<EVA>(id);
   br2.publish<EVA>();
}

} // namespace keryx
