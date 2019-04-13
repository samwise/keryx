#pragma once

#include "broker_common.h"

namespace keryx {
class Topic {
 public:
   Topic(StreamType const &type_id, StreamName const &name)
       : _stream_type_id(type_id), _stream_name(name) {}

   StreamType const &stream_type_id() const { return _stream_type_id; }
   StreamName const &producer_name() { return _stream_name; }
   ~Topic() {} // necessary because of clang bug
 private:
   StreamType _stream_type_id;
   StreamName _stream_name;
};
} // namespace keryx
