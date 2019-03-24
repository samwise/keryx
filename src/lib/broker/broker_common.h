#pragma once

#include <expected.hpp>
#include <memory>
#include <string>
#include <functional>
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
using ProducerID = uint64_t;
class ProducerImpl;
class ConsumerImpl;
class ProducerTypeDescriptor;
using ProducerTypeID = std::string;
using NotificationID = uint64_t;
using ProducerName = std::string;
class Topic;

class Notification {
public:
   NotificationKind kind;
   keryx_small_vector<EventPtr,1> const events;
   ProducerID producer_id;
   Topic const& topic;
};

using NotificationHandler = std::function<void(Notification const &)>;

class ProducerFilter {
public:
   ProducerTypeID producer_type_id;
   std::function<bool(ProducerName const&)> is_match;
   ~ProducerFilter() {} // necessary because of clang bug
};

} // namespace keryx
