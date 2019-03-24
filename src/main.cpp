#include "lib/broker/Broker.h"
#include "lib/broker/ProducerType.h"
#include "lib/broker/ProducerTypeRegistry.h"
#include "lib/broker/Topic.h"
#include "lib/broker/broker_common.h"
#include "lib/broker/Producer.h"
#include "lib/broker/Consumer.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/circular_buffer.hpp>


#include <queue>

using Timer = std::chrono::system_clock;
using TimeStamp = Timer::time_point;
using Duration = Timer::duration;

namespace keryx {
namespace bm {

struct SimpleEvent : public Event {
   SimpleEvent() : ts(Timer::now()) {}
   TimeStamp ts;
};

class SimpleProducerType : public ProducerTypeDescriptor {
 public:
   ProducerTypeID id() const override { return "SimpleProducerType"; }
   SnapshotPolicy snapshot_policy() const override {
      return SnapshotPolicy::NO_SNAPSHOT;
   }
   uint64_t serialized_event_size(Event const &) const override {
      return sizeof(SimpleEvent);
   }
   void serialize_event(Event const &, std::string_view *) const override {}
   EventPtrOrError deserialize_event(std::string_view const &) const override {
      auto e = Error{Error::DESERIALIZATION_FAILED, "Not supported"};
      return EventPtrOrError(tl::unexpected(e));
   }
   bool is_right_event_type(Event const &) const override { return true; }

   EventPtr clone_event(Event const &ev,keryx_memory_resource&mem) const override {
      auto &sev = (SimpleEvent &)ev;
      auto buffer = mem.allocate(sizeof(SimpleEvent));
      return new (buffer) SimpleEvent(sev);
   }

   void destroy_event(EventPtr ev,keryx_memory_resource&mem) const override {
      auto sev = (SimpleEvent *)&ev;
      sev->~SimpleEvent();
      mem.deallocate(sev,sizeof(SimpleEvent));
   }

   uint64_t hash_event(Event const &) const override { return 0; }
};

class MyRegistry : public ProducerTypeRegistry {
 public:
   ProducerTypeDescriptor const &get(ProducerTypeID const &) override { return simple; }
   SimpleProducerType simple;
};

static void queue_std(benchmark::State &state) {
   MyRegistry reg;
   Broker b(reg);
   Topic t{"SimpleProducerType", "test"};
   ProducerFilter f{"SimpleProducerType", [](auto const &) { return true; }};

   auto &p = b.make_producer(t);
   b.make_consumer(f, [](auto const &) {});


   boost::container::pmr::unsynchronized_pool_resource rsrc;
   boost::container::pmr::polymorphic_allocator<SimpleEvent> alloc(&rsrc);
   std::queue<EventPtr> queue;
   
   SimpleEvent ev;
   SimpleProducerType type_desc;
   for (auto _ : state) {
      queue.push(type_desc.clone_event(ev,rsrc));
      auto evp = queue.front();
      b.publish(p, evp);
      queue.pop();
      type_desc.destroy_event(evp,rsrc);
   }
}
BENCHMARK(queue_std);

static void direct(benchmark::State &state) {
   MyRegistry reg;
   Broker b(reg);
   Topic t{"SimpleProducerType", "test"};
   ProducerFilter f{"SimpleProducerType", [](auto const &) { return true; }};

   auto &p = b.make_producer(t);
   b.make_consumer(f, [](auto const &) {});


   boost::container::pmr::unsynchronized_pool_resource rsrc;
   boost::container::pmr::polymorphic_allocator<SimpleEvent> alloc(&rsrc);
   
   SimpleEvent ev;
   SimpleProducerType type_desc;
   for (auto _ : state) {
      auto evp = type_desc.clone_event(ev,rsrc);
      b.publish(p, evp);
      type_desc.destroy_event(evp,rsrc);
   }
}
BENCHMARK(direct);

static void queue_boost(benchmark::State &state) {
   MyRegistry reg;
   Broker b(reg);
   Topic t{"SimpleProducerType", "test"};
   ProducerFilter f{"SimpleProducerType", [](auto const &) { return true; }};

   auto &p = b.make_producer(t);
   b.make_consumer(f, [](auto const &) {});


   boost::lockfree::spsc_queue<EventPtr> queue(10);
   boost::container::pmr::unsynchronized_pool_resource rsrc;
   boost::container::pmr::polymorphic_allocator<SimpleEvent> alloc(&rsrc);
   
   SimpleEvent ev;
   SimpleProducerType type_desc;
   for (auto _ : state) {
      queue.push(type_desc.clone_event(ev,rsrc));
      EventPtr evp;
      queue.pop(evp);
      b.publish(p, evp);
      type_desc.destroy_event(evp,rsrc);
   }
}
BENCHMARK(queue_boost);

static void queue_ring_buffer(benchmark::State &state) {
   MyRegistry reg;
   Broker b(reg);
   Topic t{"SimpleProducerType", "test"};
   ProducerFilter f{"SimpleProducerType", [](auto const &) { return true; }};

   auto &p = b.make_producer(t);
   b.make_consumer(f, [](auto const &) {});

   boost::circular_buffer<EventPtr> queue(10);
   boost::container::pmr::unsynchronized_pool_resource rsrc;
   boost::container::pmr::polymorphic_allocator<SimpleEvent> alloc(&rsrc);
   
   SimpleEvent ev;
   SimpleProducerType type_desc;
   for (auto _ : state) {
      queue.push_back(type_desc.clone_event(ev,rsrc));
      EventPtr evp = queue.front();
      queue.pop_front();
      b.publish(p, evp);
      type_desc.destroy_event(evp,rsrc);
   }
}
BENCHMARK(queue_ring_buffer);

static void baseline(benchmark::State &state) {
   MyRegistry reg;
   Broker b(reg);
   Topic t{"SimpleProducerType", "test"};
   ProducerFilter f{"SimpleProducerType", [](auto const &) { return true; }};

   auto &p = b.make_producer(t);
   b.make_consumer(f, [](auto const &) {});

   SimpleEvent ev;
   for (auto _ : state) {
      b.publish(p, &ev);
   }
}
BENCHMARK(baseline);

static void baseline_hash_loockup(benchmark::State &state) {
   MyRegistry reg;
   Broker b(reg);
   Topic t{"SimpleProducerType", "test"};
   ProducerFilter f{"SimpleProducerType", [](auto const &) { return true; }};

   auto &p = b.make_producer(t);
   b.make_consumer(f, [](auto const &) {});

   boost::container::flat_map<int,ProducerImpl*> m;
   for (int i = 0; i< 10; i++)
      m[i] = 0;
          
   m[0] = &p;
   SimpleEvent ev;
   for (auto _ : state) {
      b.publish(*m[0], &ev);
   }
}
BENCHMARK(baseline_hash_loockup);


static void baseline_api(benchmark::State &state) {
   MyRegistry reg;
   Broker b(reg);
   Topic t{"SimpleProducerType", "test"};
   ProducerFilter f{"SimpleProducerType", [](auto const &) { return true; }};


   Consumer c {b,f,[](auto const &) {}};
   Producer p {b,t};

   SimpleEvent ev;
   for (auto _ : state) {
      p.publish(ev);
   }
}
BENCHMARK(baseline_api);

}} // namespace keryx


int main(int argc, char **argv) {
   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
   return 0;
}
