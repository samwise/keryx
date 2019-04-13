#pragma once

#include <cstdint>
#include "broker_common.h"
#include "SnapshotHandler.h"
#include "../utils/allocators.h"

namespace keryx {

class StreamDescriptor {
 public:
   virtual StreamType stream_type() const = 0;
   virtual EventPtr clone_event(Event const &,keryx_memory_resource &) const = 0;
   virtual SnapshotHandlerPtr make_snapshot_handler(keryx_memory_resource &) const = 0;
};

}
