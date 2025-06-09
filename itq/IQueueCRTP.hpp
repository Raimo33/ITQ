/*================================================================================

File: IQueueCRTP.hpp                                                            
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-06-09 20:06:35                                                

================================================================================*/

#pragma once

#include <cstddef>
#include <atomic>
#include <array>

#ifndef CACHELINE_SIZE
# define CACHELINE_SIZE 64
#endif

template <size_t N>
concept is_power_of_two = (N > 0) && ((N & (N - 1)) == 0);

namespace itq
{

template <typename Derived, typename Item, size_t Capacity>
  requires is_power_of_two<Capacity> && std::is_trivially_constructible_v<Item> && std::is_trivially_destructible_v<Item>
class IQueueCRTP
{
  public:
    IQueueCRTP(void) = default;
    ~IQueueCRTP(void) = default;

    template <typename ForwardItem>
    inline void push(ForwardItem &&item) noexcept {
      derived()->push_impl(std::forward<ForwardItem>(item));
    }

    template <typename... Args>
    inline void emplace(Args &&...args) noexcept {
      derived()->emplace_impl(std::forward<Args>(args)...);
    }

    inline bool pop(Item &out) noexcept {
      return derived()->pop_impl(out);
    }

  protected:
    static constexpr size_t _wrap_mask = Capacity - 1;

    alignas(CACHELINE_SIZE) std::array<Item, Capacity> _buffer{};

  private:
    inline Derived *derived(void) noexcept { return static_cast<Derived*>(this); }
    inline const Derived *derived(void) const noexcept { return static_cast<const Derived*>(this); }
};

}
