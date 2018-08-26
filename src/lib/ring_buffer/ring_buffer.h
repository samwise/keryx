#pragma once

#include <stdint.h>
#include <atomic>
#include <array>
#include <vector>
 
namespace keryx {

// sequence number
// size

class SlotHeader {
public:
   SlotHeader() : _data (0) {}
   
   void unpack (uint32_t *sn, uint32_t *sz) {
      auto v = _data.load();
      *sn = v >> 32;
      *sz = (uint32_t) v;
   }
   void pack (uint32_t sn, uint32_t sz) {
      uint64_t v = sn;
      sn <<= 31 ;
      v |= sz;
      _data = v;
   }
   
private:
   std::atomic<uint64_t> _data;
};

using SlotData = std::array<uint8_t, 56> ;

class Slot {
public:
   SlotHeader header;
   SlotData data;
};

class RingBuffer {
private:
   std::vector<Slot> _slots;
};

}
