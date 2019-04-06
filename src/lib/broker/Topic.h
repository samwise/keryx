#pragma once

#include "broker_common.h"

namespace keryx {
class Topic {
 public:
   Topic(StreamTypeID const &type_id, StreamName const &name)
       : _stream_type_id(type_id), _stream_name(name) {}

   StreamTypeID const &stream_type_id() const { return _stream_type_id; }
   StreamName const &producer_name() { return _stream_name; }
   ~Topic() {} // necessary because of clang bug
 private:
   StreamTypeID _stream_type_id;
   StreamName _stream_name;
};
} // namespace keryx
