#pragma once

#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/pmr/vector.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <memory>


namespace keryx {
template <class T> using keryx_pmr = boost::container::pmr::polymorphic_allocator<T>;
using keryx_memory_resource = boost::container::pmr::memory_resource;
} // namespace keryx
