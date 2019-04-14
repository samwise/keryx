#pragma once

#include "broker_common.h"

namespace keryx {
class TopicImpl {
 public:
   TopicImpl(StreamTypeID const &type_id, StreamName const &name)
       : _stream_type_id(type_id), _stream_name(name) {}

   StreamTypeID const &stream_type_id() const { return _stream_type_id; }
   StreamName const &stream_name() const { return _stream_name; }
   ~TopicImpl() {} // necessary because of clang bug
 private:
   StreamTypeID const &_stream_type_id;
   StreamName _stream_name;
};
} // namespace keryx
