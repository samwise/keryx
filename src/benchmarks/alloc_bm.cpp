#include <benchmark/benchmark.h>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>
#include <boost/container/pmr/synchronized_pool_resource.hpp>
#include <memory>
#include <chrono>

namespace keryx {
namespace bm {

using Item = std::chrono::system_clock::time_point;

static void alloc_monotonic(benchmark::State &state) {

   boost::container::pmr::monotonic_buffer_resource rsrc;
   boost::container::pmr::polymorphic_allocator<Item> alloc(&rsrc);

   for (auto _ : state) {
      alloc.deallocate(alloc.allocate(sizeof(Item)),sizeof(Item));
   }
}
BENCHMARK(alloc_monotonic);

static void alloc_pool(benchmark::State &state) {

   boost::container::pmr::unsynchronized_pool_resource rsrc;
   boost::container::pmr::polymorphic_allocator<Item> alloc(&rsrc);

   for (auto _ : state) {
      alloc.deallocate(alloc.allocate(sizeof(Item)),sizeof(Item));
   }
}
BENCHMARK(alloc_pool);

static void alloc_concurrent_pool(benchmark::State &state) {

   boost::container::pmr::synchronized_pool_resource rsrc;
   boost::container::pmr::polymorphic_allocator<Item> alloc(&rsrc);

   for (auto _ : state) {
      alloc.deallocate(alloc.allocate(sizeof(Item)),sizeof(Item));
   }
}
BENCHMARK(alloc_concurrent_pool);

static void alloc_default(benchmark::State &state) {

   boost::container::pmr::polymorphic_allocator<Item> alloc(nullptr);
   for (auto _ : state) {
      alloc.deallocate(alloc.allocate(sizeof(Item)),sizeof(Item));
   }
}
BENCHMARK(alloc_default);

static void alloc_shared_monotonic(benchmark::State &state) {

   boost::container::pmr::monotonic_buffer_resource rsrc;
   boost::container::pmr::polymorphic_allocator<Item> alloc(&rsrc);

   for (auto _ : state) {
      std::allocate_shared<Item>(alloc);
   }
}
BENCHMARK(alloc_shared_monotonic);

static void alloc_shared_pool(benchmark::State &state) {

   boost::container::pmr::unsynchronized_pool_resource rsrc;
   boost::container::pmr::polymorphic_allocator<Item> alloc(&rsrc);

   for (auto _ : state) {
      std::allocate_shared<Item>(alloc);
   }
}
BENCHMARK(alloc_shared_pool);

static void alloc_shared_default(benchmark::State &state) {

   boost::container::pmr::polymorphic_allocator<Item> alloc(nullptr);
   for (auto _ : state) {
      std::allocate_shared<Item>(alloc);
   }
}
BENCHMARK(alloc_shared_default);
}} // namespace alloc_bm
