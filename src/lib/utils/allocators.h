#pragma once

#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/container/pmr/vector.hpp>
#include <memory>

namespace keryx {
template <class T>
using keryx_pmr = boost::container::pmr::polymorphic_allocator<T>;
using keryx_memory_resource = boost::container::pmr::memory_resource;

// While we wait for std::allocate_unique
template <typename Alloc> struct alloc_deleter {
   alloc_deleter(const Alloc &a) : a(a) {}

   typedef typename std::allocator_traits<Alloc>::pointer pointer;

   void operator()(pointer p) const {
      Alloc aa(a);
      std::allocator_traits<Alloc>::destroy(aa, std::addressof(*p));
      std::allocator_traits<Alloc>::deallocate(aa, p, 1);
   }

 private:
   Alloc a;
};

template <typename T, typename... Args>
auto keryx_allocate_unique(const keryx_pmr<T> &alloc, Args &&... args) {
   using Alloc = keryx_pmr<T>;
   using AT = std::allocator_traits<Alloc>;
   static_assert(std::is_same<typename AT::value_type, std::remove_cv_t<T>>{}(),
                 "Allocator has the wrong value_type");

   Alloc a(alloc);
   auto p = AT::allocate(a, 1);
   try {
      AT::construct(a, std::addressof(*p), std::forward<Args>(args)...);
      using D = alloc_deleter<Alloc>;
      return std::unique_ptr<T, D>(p, D(a));
   } catch (...) {
      AT::deallocate(a, p, 1);
      throw;
   }
}

template <class T> using keryx_unique_ptr = std::unique_ptr<T,alloc_deleter<keryx_pmr<T>>>;

} // namespace keryx
