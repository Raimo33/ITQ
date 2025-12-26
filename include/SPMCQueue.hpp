/*================================================================================

File: SPMCQueue.hpp                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-12-26 16:48:41                                                

================================================================================*/

#pragma once

#include "common.hpp"

namespace itq
{

namespace spmc
{

template <typename Item, size_t Capacity>
  requires is_power_of_two<Capacity>
struct SharedData
{
  alignas(CACHELINE_SIZE) std::atomic<size_t> write_idx{0};
  alignas(CACHELINE_SIZE) std::atomic<size_t> read_idx{0};
  alignas(CACHELINE_SIZE) std::array<Item, Capacity> buffer{};
};

template <typename Item, size_t Capacity>
class Producer
{
  public:
    Producer(SharedData<Item, Capacity> &data) noexcept :
      _data(data)
    {}

    template <typename ForwardItem>
    void push(ForwardItem &&item) noexcept
    {
      _data.buffer[_local_write_idx] = std::forward<ForwardItem>(item);
      _local_write_idx = (_local_write_idx + 1) & _wrap_mask;
      _data.write_idx.store(_local_write_idx, std::memory_order_release);
    }

    template <typename... Args>
    void emplace(Args &&...args) noexcept
    {
      new (&_data.buffer[_local_write_idx]) Item(std::forward<Args>(args)...);
      _local_write_idx = (_local_write_idx + 1) & _wrap_mask;
      _data.write_idx.store(_local_write_idx, std::memory_order_release);
    }

  private:
    static constexpr size_t _wrap_mask = Capacity - 1;

    alignas(CACHELINE_SIZE) std::byte _pad_barrier{};
    SharedData<Item, Capacity> &_data;
    size_t _local_write_idx{0};
};

template <typename Item, size_t Capacity>
class Consumer
{
  public:
    Consumer(SharedData<Item, Capacity> &data) noexcept :
      _data(data)
    {}

    bool pop(Item &out) noexcept
    {
      size_t local_read_idx = _data.read_idx.load(std::memory_order_relaxed);

      while (true)
      {
        if (local_read_idx == _cached_write_idx) [[unlikely]]
        {
          _cached_write_idx = _data.write_idx.load(std::memory_order_acquire);
          if (local_read_idx == _cached_write_idx) [[unlikely]]
            return false;
        }

        if (_data.read_idx.compare_exchange_weak(local_read_idx, (local_read_idx + 1) & _wrap_mask, std::memory_order_acquire, std::memory_order_relaxed)) [[likely]]
        {
          out = std::move(_data.buffer[local_read_idx]);
          std::destroy_at(&_data.buffer[local_read_idx]);
          return true;
        }
      }

      std::unreachable();
    }

  private:
    static constexpr size_t _wrap_mask = Capacity - 1;

    alignas(CACHELINE_SIZE) std::byte _pad_barrier;
    SharedData<Item, Capacity> &_data;
    size_t _cached_write_idx{0};
};

} // namespace spmc
} // namespace itq
