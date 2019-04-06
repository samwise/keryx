#include "SnapshotHandler.h"

namespace keryx {

class NoSnapshot : public SnapshotHandler {
 public:
   void add_new_event(uint64_t, EventPtr const &) override {}
   std::vector<EventPtr> const &get_snapshot() override { return _empty; }

 private:
   std::vector<EventPtr> _empty;
};

class CacheLast : public SnapshotHandler {
 public:
   void add_new_event(uint64_t, EventPtr const &ev) override {
      _last.clear();
      _last.push_back(ev);
   }
   std::vector<EventPtr> const &get_snapshot() override { return _last; }

 private:
   std::vector<EventPtr> _last;
};

class CacheAll : public SnapshotHandler {
 public:
   void add_new_event(uint64_t, EventPtr const &ev) override {
      _all.push_back(ev);
   }
   std::vector<EventPtr> const &get_snapshot() override { return _all; }

 private:
   std::vector<EventPtr> _all;
};

class CacheHash : public SnapshotHandler {
 public:
   void add_new_event(uint64_t h, EventPtr const &ev) override {
      _hash[h] = ev;
      _snapshot.clear();
   }

   std::vector<EventPtr> const &get_snapshot() override {
      if (_snapshot.size() || _hash.empty())
         return _snapshot;
      else {
         for (auto const &kvp : _hash)
            _snapshot.push_back(kvp.second);
         return _snapshot;
      }
   }

 private:
   std::vector<EventPtr> _snapshot;
   std::unordered_map<uint64_t, EventPtr> _hash;
};

std::shared_ptr<SnapshotHandler> make_snapshot_handler(SnapshotPolicy policy) {
   switch (policy) {
   case SnapshotPolicy::NO_SNAPSHOT:
      return std::make_unique<NoSnapshot>();
   case SnapshotPolicy::CACHE_LAST:
      return std::make_unique<CacheLast>();
   case SnapshotPolicy::CACHE_ALL:
      return std::make_unique<CacheAll>();
   case SnapshotPolicy::CACHE_HASH:
      return std::make_unique<CacheHash>();
   }
   return std::unique_ptr<SnapshotHandler> ();
}

} // namespace keryx
