#pragma once

#include <expected.hpp>
#include <memory>
#include <string>
#include <functional>

namespace keryx {

class Event {};

struct Error {
   enum ErrorCode { DESERIALIZATION_FAILED };
   ErrorCode code;
   std::string msg;
};

enum class SnapshotPolicy { NO_SNAPSHOT, CACHE_LAST, CACHE_ALL, CACHE_HASH };
enum class NotificationKind { START_PRODUCER,EVENT,STOP_PRODUCER };

using EventPtr = std::shared_ptr<const Event>;
using EventPtrOrError = tl::expected<EventPtr, Error>;
using ProducerID = uint64_t;
class Producer;
class Consumer;
class ProducerType;
using ProducerTypeID = std::string;
using NotificationID = uint64_t;
using ProducerName = std::string;
class Topic;

class Notification {
public:
   NotificationKind kind;
   std::vector<EventPtr> const &events;
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
