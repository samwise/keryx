#include "lib/broker/Broker.h"
#include "lib/broker/Consumer.h"
#include "lib/broker/ConsumerImpl.h"
#include "lib/broker/Producer.h"
#include "lib/broker/ProducerImpl.h"
#include "lib/broker/StreamDescriptor.h"
#include "lib/broker/StreamDescriptorRegistry.h"
#include "lib/broker/Topic.h"
#include "lib/broker/broker_common.h"

#include <benchmark/benchmark.h>
#include <boost/circular_buffer.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <chrono>
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

class SimpleDescriptor : public StreamDescriptor {
 public:
   StreamType stream_type() const override { return "Simple"; }

   EventPtr clone_event(Event const &ev,
                        keryx_memory_resource &mem) const override {
      auto &sev = (SimpleEvent &)ev;
      auto buffer = mem.allocate(sizeof(SimpleEvent));
      return {new (buffer) SimpleEvent(sev), [&mem](Event *ev) {
                 auto sev = (SimpleEvent *)ev;
                 sev->~SimpleEvent();
                 mem.deallocate(sev, sizeof(SimpleEvent));
              }};
   }

   SnapshotHandlerPtr
   make_snapshot_handler(keryx_memory_resource &mem) const override {
      auto buffer = mem.allocate(sizeof(NullSnapshotHandler));
      return {new (buffer) NullSnapshotHandler(), [&mem](SnapshotHandler *h) {
                 h->~SnapshotHandler();
                 mem.deallocate(h, sizeof(NullSnapshotHandler));
              }};
   }
};

class MyRegistry : public StreamDescriptorRegistry {
 public:
   StreamDescriptor const &get(StreamType const &) override { return simple; }
   SimpleDescriptor simple;
};

static void alloc(benchmark::State &state) {
   boost::container::pmr::unsynchronized_pool_resource rsrc;

   SimpleDescriptor desc;
   SimpleEvent ev;
   for (auto _ : state) {
      auto p = desc.clone_event(ev, rsrc);
      p.reset();
   }
}
BENCHMARK(alloc);

static void publish(benchmark::State &state) {
   boost::container::pmr::unsynchronized_pool_resource rsrc;

   MyRegistry reg;
   Broker b(reg, rsrc);
   Topic t{"Simple", "test"};
   StreamFilter f{"SimpleProducerType", [](auto const &) { return true; }};

   Producer p{b, t};
   Consumer c{b, f, [](auto const &) {}};

   SimpleEvent ev;
   for (auto _ : state) {
      p.publish(ev);
      b.do_work();
   }
}
BENCHMARK(publish);

void vg_workout() {
   boost::container::pmr::unsynchronized_pool_resource rsrc;

   MyRegistry reg;
   Broker b(reg, rsrc);
   Topic t{"Simple", "test"};
   StreamFilter f{"SimpleProducerType", [](auto const &) { return true; }};

   Producer p{b, t};
   Consumer c{b, f, [](auto const &) {}};

   SimpleEvent ev;
   for (;;) {
      p.publish(ev);
      b.do_work();
   }
}

} // namespace bm
} // namespace keryx

int main(int argc, char **argv) {
   if (argc > 1)
      keryx::bm::vg_workout();
   else {
      benchmark::Initialize(&argc, argv);
      benchmark::RunSpecifiedBenchmarks();
   }
   return 0;
}
