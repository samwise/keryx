#pragma once

#include <array>
#include <atomic>
#include <stdint.h>
#include <vector>
#include <cassert>
#include <string_view>
#include <cstring>

namespace keryx {

// sequence number
// size

struct SlotHeader {
   uint32_t sn;
   uint32_t sz;
};

using SlotData = std::array<uint8_t, 56>;

class Slot {
 public:
   Slot() : _header{SlotHeader{0, 0}}, _data() {}

   SlotHeader header() const { return _header; }
   SlotData const &data() const { return _data; }

   void update (std::string_view const&d,uint32_t sn) {
      assert(d.size() < sizeof(SlotData));
      
      _header.store(SlotHeader {0,0});
      memcpy(&_data[0],d.data(),d.size());
      _header.store(SlotHeader{sn,(uint32_t) d.size()});
   }
  
 private:
   std::atomic<SlotHeader> _header;
   SlotData _data;
};

enum class WriteResult {
   OK, MSG_TOO_BIG, EMPTY_MSG 
};

class RingBuffer {
 public:
   RingBuffer(size_t n_slots = 1000) :
      _current_slot(0),
      _current_sn(1),
      _slots(n_slots)
   {
      assert(n_slots > 0);
   }

   WriteResult write(std::string_view const&sview) {
      if (sview.size() > _slots.size() * sizeof(SlotData))
         return WriteResult::MSG_TOO_BIG;
      else if (sview.empty())
         return WriteResult::EMPTY_MSG;
      else {
         size_t current_byte = 0;
         while (current_byte != sview.size()) {
            auto remaining = sview.size() - current_byte;
            auto to_write = std::min(sizeof(SlotData), remaining);
            auto &cslot = _slots[_current_slot];
            cslot.update(std::string_view(&sview[current_byte],to_write),_current_sn);
            current_byte+= to_write;
            move_next();
         }
         return WriteResult::OK;
      }
   }

 private:
   void move_next() {
      _current_sn++;
      _current_slot++;
      if (_current_slot == _slots.size())
         _current_slot = 0;
   }

   friend class RingBufferReader;
   uint32_t _current_slot;
   uint32_t _current_sn;
   std::vector<Slot> _slots;
};

class ReadResult {
 public:
   enum Result { OK, NO_DATA, LOSS };

   Result result() const { return _result; }
   std::string const &data() const { return _data; }

   void clear() { _data.clear(); }
   void append(SlotData const&data,uint32_t sz) {
      if (sz != 0)
         _data.append((const char*) &data[0], sz);
      else
         _data.append((const char*) &data[0],sizeof(SlotData));
   }
   void set_result(Result r) { _result = r; }

 private:
   Result _result;
   std::string _data;
};

class RingBufferReader {
 public:
   RingBufferReader(RingBuffer &buffer)
       : _buffer(buffer), _expected_sn(1), _current_slot(0) {}

   ReadResult const&read_next() {
      auto w_esn = _expected_sn;
      auto w_cs = _current_slot;
      _result.clear();
      
      for (;;) {
         auto const &cslot = _buffer._slots[_current_slot];
         auto h = cslot.header();

         if (h.sn == 0) {
            _result.set_result(ReadResult::Result::NO_DATA);
            break;
         }

         if (h.sn != _expected_sn) {
            _result.set_result(ReadResult::Result::LOSS);
            break;
         }

         _result.append(cslot.data(),h.sz );
         move_next(&w_esn,&w_cs);

         auto h_now = cslot.header();
         if (h.sn != h_now.sn) {
            _result.set_result(ReadResult::Result::LOSS);
            break;
         }

         if (h_now.sz) {
            _expected_sn = w_esn;
            _current_slot = w_cs;
            _result.set_result(ReadResult::Result::OK);
            break;
         }
      }
 
      return _result;
   }
   
 private:
   void move_next(uint32_t *sn, uint32_t *cs) {
      *sn = (*sn) + 1;
      *cs = (*cs) + 1;
      
      if (*cs == _buffer._slots.size())
         *cs = 0;
   }
   
   RingBuffer &_buffer;
   uint32_t _expected_sn;
   uint32_t _current_slot;
   ReadResult _result;
};

} // namespace keryx
