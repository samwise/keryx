#pragma once

#include <expected.hpp>
#include <memory>
#include <string>
#include <functional>
#include <queue>
#include <deque>
#include <boost/container/pmr/small_vector.hpp>

#include "../utils/allocators.h"
namespace keryx {

class Event {};

struct Error {
   enum ErrorCode { DESERIALIZATION_FAILED };
   ErrorCode code;
   std::string msg;
};

enum class SnapshotPolicy { NO_SNAPSHOT, CACHE_LAST, CACHE_ALL, CACHE_HASH };
enum class NotificationKind { START_PRODUCER,EVENT,STOP_PRODUCER };

using EventPtr = Event const*;
using EventPtrOrError = tl::expected<EventPtr, Error>;
using StreamID = uint64_t;
class ProducerImpl;
class ConsumerImpl;
class StreamDescriptor;
using StreamTypeID = std::string;
using NotificationID = uint64_t;
using StreamName = std::string;
class Topic;

using ProducerImplPtr = std::shared_ptr<ProducerImpl>;
using ConsumerImplPtr = std::shared_ptr<ConsumerImpl>;
template <class T> using keryx_vec = std::vector<T, keryx_pmr<T>>;
template <class T> using keryx_queue = std::queue<T, std::deque<T,keryx_pmr<T>>>;

class Notification {
public:
   NotificationKind kind;
   keryx_vec<EventPtr> const events;
   StreamID stream_id;
   Topic const& topic;
};

using NotificationHandler = std::function<void(Notification const &)>;

class StreamFilter {
public:
   StreamTypeID stream_type_id;
   std::function<bool(StreamName const&)> is_match;
   ~StreamFilter() {} // necessary because of clang bug
};


} // namespace keryx
