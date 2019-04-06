#pragma once
#include "broker_common.h"

namespace keryx {

class SnapshotHandler {
 public:
   virtual ~SnapshotHandler() {}
   virtual void add_new_event(uint64_t event_hash, EventPtr const &ev) = 0;
   virtual std::vector<EventPtr> const &get_snapshot() = 0;
};

std::shared_ptr<SnapshotHandler> make_snapshot_handler(SnapshotPolicy policy) ;

}
