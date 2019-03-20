#pragma once

#include <boost/container/pmr/vector.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/pmr/small_vector.hpp>
#include <boost/container/small_vector.hpp>

namespace keryx {
using keryx_memory_resource = boost::container::pmr::memory_resource;
template <class T> using keryx_pmr_vector = boost::container::pmr::vector<T>;
template <class T> using keryx_pmr_vector = boost::container::pmr::vector<T>;
template <class T,size_t SZ> using keryx_pmr_small_vector = boost::container::pmr::small_vector<T,SZ>;
template <class T,size_t SZ> using keryx_small_vector = boost::container::small_vector<T,SZ>;
}
