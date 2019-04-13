#pragma once

#include <expected.hpp>
#include <memory>
#include <string>
#include <functional>
#include <queue>
#include <deque>
#include <boost/container/small_vector.hpp>

#include "../utils/allocators.h"
namespace keryx {

class Event {
public:
};

struct Error {
   enum ErrorCode { DESERIALIZATION_FAILED };
   ErrorCode code;
   std::string msg;
};

enum class NotificationKind { START_PRODUCER,EVENT,STOP_PRODUCER };

using EventPtr = std::unique_ptr<Event,std::function<void(Event*)>>;
using EventPtrOrError = tl::expected<EventPtr, Error>;
using StreamID = uint64_t;
class ProducerImpl;
class ConsumerImpl;
class StreamDescriptor;
using StreamType = std::string;
using NotificationID = uint64_t;
using StreamName = std::string;
class Topic;

using ProducerImplPtr = keryx_unique_ptr<ProducerImpl>;
using ConsumerImplPtr = keryx_unique_ptr<ConsumerImpl>;
template <class T> using keryx_vec = std::vector<T, keryx_pmr<T>>;
template <class T,size_t N> using keryx_small_vec = boost::container::small_vector<T,N>;
class Notification {
public:
   NotificationKind kind;
   keryx_small_vec<Event const*,1> const events;
   StreamID stream_id;
   Topic const& topic;
};

using NotificationHandler = std::function<void(Notification const &)>;

class StreamFilter {
public:
   StreamType stream_type;
   std::function<bool(StreamName const&)> is_match;
   ~StreamFilter() {} // necessary because of clang bug
};


} // namespace keryx
