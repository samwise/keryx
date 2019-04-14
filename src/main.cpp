#include "lib/broker/Broker.h"
#include "lib/broker/Consumer.h"
#include "lib/broker/Producer.h"
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

template <class ET>
class SimpleStream {
 public:
   using EventType = ET;
   
   static EventPtr clone_event(ET const &sev,
                        keryx_memory_resource &mem)  {
      auto buffer = mem.allocate(sizeof(ET));
      return {new (buffer) ET(sev), [&mem](Event *ev) {
                 auto sev = (ET *)ev;
                 sev->~ET();
                 mem.deallocate(sev, sizeof(ET));
              }};
   }

   static SnapshotHandlerPtr
   make_snapshot_handler(keryx_memory_resource &mem) {
      auto buffer = mem.allocate(sizeof(NullSnapshotHandler));
      return {new (buffer) NullSnapshotHandler(), [&mem](SnapshotHandler *h) {
                 h->~SnapshotHandler();
                 mem.deallocate(h, sizeof(NullSnapshotHandler));
              }};
   }
};

static void alloc(benchmark::State &state) {
   boost::container::pmr::unsynchronized_pool_resource rsrc;

   SimpleEvent ev;
   for (auto _ : state) {
      auto p = SimpleStream<SimpleEvent>::clone_event(ev, rsrc);
      p.reset();
   }
}
BENCHMARK(alloc);

static void publish(benchmark::State &state) {
   boost::container::pmr::unsynchronized_pool_resource rsrc;
   Broker b(rsrc);

   using Stream = SimpleStream<SimpleEvent>;
   Producer<Stream> p{b};
   Consumer<Stream> c{b, [](auto const &) {}};

   SimpleEvent ev;
   for (auto _ : state) {
      p.publish(ev);
      b.do_work();
   }
}
BENCHMARK(publish);

void vg_workout() {
   boost::container::pmr::unsynchronized_pool_resource rsrc;
   Broker b(rsrc);

   using Stream = SimpleStream<SimpleEvent>;
   Producer<Stream> p{b};
   Consumer<Stream> c{b, [](auto const &) {}};

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
