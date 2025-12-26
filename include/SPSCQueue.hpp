/*================================================================================

File: SPSCQueue.hpp                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-12-26 17:07:36                                                

================================================================================*/

#pragma once

#include "common.hpp"

namespace itq
{

namespace spsc
{

template <typename Item, size_t Capacity>
  requires is_power_of_two<Capacity>
struct SharedData
{
  alignas(CACHELINE_SIZE) std::atomic<size_t> write_idx{0};
  alignas(CACHELINE_SIZE) std::array<Item, Capacity> buffer{};
};

template <typename Item, size_t Capacity>
  requires is_power_of_two<Capacity>
class Producer
{
  public:
    explicit Producer(SharedData<Item, Capacity> &data) noexcept :
      _data(data)
    {}

    ~Producer(void) = default;

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
  requires is_power_of_two<Capacity>
class Consumer
{
  public:
    explicit Consumer(SharedData<Item, Capacity> &data) noexcept :
      _data(data)
    {}

    bool pop(Item &out) noexcept
    {
      if (_local_read_idx == _cached_write_idx) [[unlikely]]
      {
        _cached_write_idx = _data.write_idx.load(std::memory_order_acquire);
        if (_local_read_idx == _cached_write_idx) [[unlikely]]
          return false;
      }

      out = std::move(_data.buffer[_local_read_idx]);
      std::destroy_at(&_data.buffer[_local_read_idx]);
      _local_read_idx = (_local_read_idx + 1) & _wrap_mask;
      return true;
    }

  private:
    static constexpr size_t _wrap_mask = Capacity - 1;

    alignas(CACHELINE_SIZE) std::byte _pad_barrier;
    SharedData<Item, Capacity> &_data;
    size_t _local_read_idx{0};
    size_t _cached_write_idx{0};
};

} // namespace spsc
} // namespace itq
