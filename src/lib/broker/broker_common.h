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
enum class MessageType { START_PRODUCER,EVENT,STOP_PRODUCER };

using EventPtr = std::shared_ptr<const Event>;
using EventPtrOrError = tl::expected<EventPtr, Error>;
using ProducerID = uint64_t;
class Producer;
class Consumer;
class IProducerTypeDescriptor;
using ProducerTypeID = std::string;
   
using Topic = std::string;
using MessageID = uint64_t;

class Message {
public:
   MessageType type;
   std::vector<EventPtr> const &events;
   ProducerID producer;
   MessageID message_id;
   Topic const& topic;
   IProducerTypeDescriptor const &producer_type;
};

using MessageHandler = std::function<void(Message const &)>;

class ProducerFilter {
public:
   ProducerTypeID producer_type_id;
   std::function<bool(Topic const&)> is_match;
};

} // namespace keryx
