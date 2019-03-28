#pragma once

#include <cstdint>
#include "broker_common.h"
#include "../utils/allocators.h"

namespace keryx {

class ProducerTypeDescriptor {
 public:
   virtual ProducerTypeID id() const = 0;
   virtual SnapshotPolicy snapshot_policy() const = 0;
   virtual uint64_t serialized_event_size(Event const &) const = 0;
   virtual void serialize_event(Event const &, std::string_view *out) const = 0;
   virtual EventPtrOrError deserialize_event(std::string_view const &bytes) const = 0;
   virtual bool is_right_event_type(Event const &) const = 0;
   virtual EventPtr clone_event(Event const &,keryx_memory_resource &rsrc) const = 0;
   virtual void destroy_event(EventPtr,keryx_memory_resource &rsrc) const = 0;
   virtual uint64_t hash_event(Event const &) const = 0;
};

}