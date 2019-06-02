#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace keryx {

class IStream;
class IStreamDesc;
class EventPtr;
struct StreamFilter;
class Notification;
using EventHandler = std::function<void(Notification const&)>;
class SerializedNotification;

using NodeID = uint32_t;
using StreamID = uint32_t;
using StreamID = uint32_t;
using StreamName = std::string;
using StreamType = std::string;
using SubID = uint32_t;

class INode {
 public:
   virtual void advertise_remote_stream(NodeID, StreamID, StreamName,
                                        IStreamDesc &) = 0;
   virtual void unadvertise_remote_stream(StreamID) = 0;
   virtual void add_remote_subscription(NodeID,StreamID, SubID) = 0;
   virtual void remove_remote_subscription(NodeID,StreamID) = 0;
   virtual void remote_notify(StreamID, SubID, SerializedNotification &) = 0;
};

} // namespace keryx
