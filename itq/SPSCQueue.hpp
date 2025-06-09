/*================================================================================

File: SPSCQueue.hpp                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-06-09 20:06:35                                                

================================================================================*/

#pragma once

#include "IQueueCRTP.hpp"

namespace itq
{

template <typename Item, size_t Capacity>
class SPSCQueue : public IQueueCRTP<SPSCQueue<Item, Capacity>, Item, Capacity>
{
  using Base = IQueueCRTP<SPSCQueue<Item, Capacity>, Item, Capacity>;
  using Base::_buffer;
  using Base::_wrap_mask;

  public:
    SPSCQueue(void) = default;

    template <typename ForwardItem>
    void push_impl(ForwardItem &&item) noexcept
    {
      _buffer[_local_write_idx & wrap_mask] = std::forward<ForwardItem>(item);
      _write_idx.store(++_local_write_idx, std::memory_order_release);
    }

    template <typename... Args>
    void emplace_impl(Args &&...args) noexcept
    {
      new (&_buffer[_local_write_idx & wrap_mask]) Item(std::forward<Args>(args)...);
      _write_idx.store(++_local_write_idx, std::memory_order_release);
    }

    bool pop_impl(Item &out) noexcept
    {
      if (_local_read_idx == _cached_write_idx) [[unlikely]]
      {
        _cached_write_idx = _write_idx.load(std::memory_order_acquire);
        if (_local_read_idx == _cached_write_idx) [[unlikely]]
          return false;
      }

      const size_t idx = _local_read_idx & _wrap_mask;
      out = std::move(_buffer[idx]);
      std::destroy_at(&_buffer[idx]);
      _local_read_idx++;
      return true;
    }

  private:
    alignas(CACHELINE_SIZE) std::atomic<size_t> _write_idx{0};
    size_t _local_read_idx{0};
    size_t _local_write_idx{0};
    size_t _cached_write_idx{0};
};

}