#include "lib/broker/Broker.h"
#include "lib/broker/ProducerType.h"
#include "lib/broker/ProducerTypeRegistry.h"
#include "lib/broker/Topic.h"
#include "lib/broker/broker_common.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>

using Timer = std::chrono::system_clock;
using TimeStamp = Timer::time_point;
using Duration = Timer::duration;

namespace keryx {

struct SimpleEvent : public Event {
   SimpleEvent() : ts(Timer::now()) {}
   TimeStamp ts;
};

class SimpleProducerType : public ProducerType {
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
   ProducerType const &get(ProducerTypeID const &) override { return simple; }
   SimpleProducerType simple;
};

static void test_monotonic(benchmark::State &state) {

   boost::container::pmr::monotonic_buffer_resource rsrc(10000000 * sizeof(SimpleEvent));
   boost::container::pmr::polymorphic_allocator<SimpleEvent> alloc(&rsrc);

   for (auto _ : state) {
      alloc.deallocate(alloc.allocate(sizeof(SimpleEvent)),1);
   }
}
BENCHMARK(test_monotonic);

static void test_pool(benchmark::State &state) {

   boost::container::pmr::unsynchronized_pool_resource rsrc;
   boost::container::pmr::polymorphic_allocator<SimpleEvent> alloc(&rsrc);

   for (auto _ : state) {
      alloc.deallocate(alloc.allocate(sizeof(SimpleEvent)),1);
   }
}
BENCHMARK(test_pool);

static void test_default(benchmark::State &state) {

   boost::container::pmr::polymorphic_allocator<SimpleEvent> alloc(nullptr);
   for (auto _ : state) {
      alloc.deallocate(alloc.allocate(sizeof(SimpleEvent)),1);
   }
}
BENCHMARK(test_default);

static void test_shared_monotonic(benchmark::State &state) {

   boost::container::pmr::monotonic_buffer_resource rsrc(10000000 * sizeof(SimpleEvent));
   boost::container::pmr::polymorphic_allocator<SimpleEvent> alloc(&rsrc);

   for (auto _ : state) {
      std::allocate_shared<SimpleEvent> (alloc);
   }
}
BENCHMARK(test_shared_monotonic);

static void test_shared_pool(benchmark::State &state) {

   boost::container::pmr::unsynchronized_pool_resource rsrc;
   boost::container::pmr::polymorphic_allocator<SimpleEvent> alloc(&rsrc);

   for (auto _ : state) {
      std::allocate_shared<SimpleEvent> (alloc);
   }
}
BENCHMARK(test_shared_pool);

static void test_shared_default(benchmark::State &state) {

   boost::container::pmr::polymorphic_allocator<SimpleEvent> alloc(nullptr);
   for (auto _ : state) {
      std::allocate_shared<SimpleEvent> (alloc);
   }
}
BENCHMARK(test_shared_default);

static void test(benchmark::State &state) {
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
   auto evp = type_desc.clone_event(ev,rsrc);
   for (auto _ : state) {
      b.publish(p, evp);
   }
   type_desc.destroy_event(evp,rsrc);
}
BENCHMARK(test);

void valgrind_workout()
{
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
   while (true) {
      auto evp = type_desc.clone_event(ev,rsrc);
      b.publish(p, evp);
      type_desc.destroy_event(evp,rsrc);
   }
}

} // namespace keryx


int main(int argc, char **argv) {
   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
   return 0;
}
