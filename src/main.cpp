#include "lib/broker/Broker.h"
#include "lib/broker/Broker2.h"
#include "lib/broker/Broker3.h"
#include "lib/utils/mp_utils.h"
#include <benchmark/benchmark.h>
#include <iostream>

namespace keryx2 {
struct EVA : public keryx::EventBase {
   int x;
   void update(int v) { x = v; }
};

static void ping_ping_k2(benchmark::State &state) {
   using BR = Broker<EVA>;
   Broker<EVA> b1;
   BR br1;
   BR br2;
   br1.add_neighbour({&br2});
   br2.add_neighbour({&br1});
   br2.subscribe<EVA>([&](auto const &) {});
   for (auto _ : state) {
      br1.publish<EVA>(1);
      br2.do_work();
   }
}
BENCHMARK(ping_ping_k2);
} // namespace keryx2

namespace keryx3 {
struct EVA {int x;};

static void ping_ping_k3(benchmark::State &state) {
   using BR = Broker<EVA>;
   Broker<EVA> b1;
   BR br1;
   BR br2;
   br1.add_neighbour({&br2});
   br2.add_neighbour({&br1});
   br2.subscribe<EVA>([&](auto const &) {});
   for (auto _ : state) {
      br1.publish<EVA>(EVA{1});
      br2.do_work();
   }
}
BENCHMARK(ping_ping_k3);
} // namespace keryx2

namespace keryx {
struct EVA : public keryx::EventBase {
   int x;
   void update(int v) { x = v; }
};

static void ping_ping(benchmark::State &state) {
   using BR = Broker<EVA>;
   Broker<EVA> b1;
   BR br1;
   BR br2;
   br1.add_neighbour({&br2});
   br2.add_neighbour({&br1});
   br2.subscribe<EVA>([&](auto const &) {});
   for (auto _ : state) {
      br1.publish<EVA>(1);
      br2.do_work();
   }
}
BENCHMARK(ping_ping);
} // namespace keryx

void bench(int argc, char **argv) {
   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
}


int main(int argc, char **argv) {
   if (argc > 1)
      bench(argc,argv);
   else {
      using namespace keryx2;
      using BR = Broker<EVA>;
      Broker<EVA> b1;
      BR br1;
      BR br2;
      br1.add_neighbour({&br2});
      br2.add_neighbour({&br1});
      br2.subscribe<EVA>([&](auto const &) {});
      while (true) {
         br1.publish<EVA>(1);
         br2.do_work();
      }
   }
   return 0;
}
