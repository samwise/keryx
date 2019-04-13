#pragma once

#include "../utils/allocators.h"
#include "broker_common.h"

namespace keryx {

class SnapshotHandler {
 public:
   virtual ~SnapshotHandler() {}
   virtual void add_new_event(EventPtr &&) = 0;
   virtual std::vector<Event const *> const &get_snapshot() = 0;
};

class NullSnapshotHandler : public SnapshotHandler {
 public:
   ~NullSnapshotHandler() {}
   void add_new_event(EventPtr &&ev) override { ev.reset(); }
   std::vector<Event const *> const &get_snapshot() override { return _; }
private:
   std::vector<Event const*> _;
};

using SnapshotHandlerPtr = std::unique_ptr<SnapshotHandler,std::function<void(SnapshotHandler*)>>;

} // namespace keryx
