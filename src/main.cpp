#include "lib/broker/Broker.h"
#include "lib/broker/ProducerType.h"
#include "lib/broker/ProducerTypeRegistry.h"
#include "lib/broker/Topic.h"
#include "lib/broker/broker_common.h"
#include "lib/broker/Producer.h"
#include "lib/broker/Consumer.h"
#include "lib/broker/ProducerImpl.h"
#include "lib/broker/ConsumerImpl.h"

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

static void publish(benchmark::State &state) {
   MyRegistry reg;
   Broker b(reg);
   Topic t{"SimpleProducerType", "test"};
   ProducerFilter f{"SimpleProducerType", [](auto const &) { return true; }};

   auto &p = b.add_producer(*new ProducerImpl(), t, {});
   b.add_consumer(*new ConsumerImpl(), f, [](auto const &) {});

   boost::container::pmr::unsynchronized_pool_resource rsrc;
   boost::container::pmr::polymorphic_allocator<SimpleEvent> alloc(&rsrc);
   
   SimpleEvent ev;
   SimpleProducerType type_desc;
   for (auto _ : state) {
      auto evp = type_desc.clone_event(ev,rsrc);
      b.publish(p, evp);
      b.do_work(); 
      type_desc.destroy_event(evp,rsrc);
   }
}
BENCHMARK(publish);

}} // namespace keryx


int main(int argc, char **argv) {
   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
   return 0;
}
