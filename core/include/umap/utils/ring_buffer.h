// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Ring buffer (fixed-size, zero-allocation)

#ifndef UMAP_UTILS_RING_BUFFER_H_
#define UMAP_UTILS_RING_BUFFER_H_

#include <array>
#include <cstddef>
#include <optional>

namespace umap::utils {

template<typename T, std::size_t N>
class RingBuffer {
    static_assert(N > 0U && N <= 4096U, "Ring buffer size must be > 0 and <= 4096");

 public:
    RingBuffer() noexcept = default;

    void push(const T& item) noexcept {
        data_[tail_] = item;
        tail_ = (tail_ + 1U) % N;
        if (count_ < N) {
            ++count_;
        } else {
            head_ = (head_ + 1U) % N;
        }
    }

    [[nodiscard]] std::optional<T> pop() noexcept {
        if (count_ == 0U) {
            return std::nullopt;
        }
        T value = data_[head_];
        head_ = (head_ + 1U) % N;
        --count_;
        return value;
    }

    [[nodiscard]] bool empty() const noexcept { return count_ == 0U; }
    [[nodiscard]] bool full() const noexcept { return count_ == N; }
    [[nodiscard]] std::size_t size() const noexcept { return count_; }

 private:
    std::array<T, N> data_{};
    std::size_t head_ = 0U;
    std::size_t tail_ = 0U;
    std::size_t count_ = 0U;
};

}  // namespace umap::utils

#endif  // UMAP_UTILS_RING_BUFFER_H_
