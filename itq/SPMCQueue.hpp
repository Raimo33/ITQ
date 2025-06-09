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
      size_t local_read_idx = _read_idx.load(std::memory_order_relaxed);

      while (true)
      {
        if (local_read_idx == _cached_write_idx) [[unlikely]]
        {
          _cached_write_idx = data->write_idx.load(std::memory_order_acquire);
          if (local_read_idx == _cached_write_idx) [[unlikely]]
            return false;
        }

        if (_read_idx.compare_exchange_weak(local_read_idx, local_read_idx + 1, std::memory_order_acquire, std::memory_order_relaxed)) [[likely]]
        {
          const size_t idx = local_read_idx & _wrap_mask;
          out = std::move(_buffer[idx]);
          std::destroy_at(&_buffer[idx]);
          return true;
        }

        std::this_thread::yield();
      }

      std::unreachable();
    }

  private:
    alignas(CACHELINE_SIZE) std::atomic<size_t> _write_idx{0};
    alignas(CACHELINE_SIZE) std::atomic<size_t> _read_idx{0};
    size_t _local_write_idx{0};
    size_t _cached_write_idx{0};
};

}