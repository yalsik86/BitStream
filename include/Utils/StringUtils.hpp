#pragma once

#include <algorithm>
#include <array>
#include <string_view>
#include <cstddef>

namespace StringUtils {
    template<std::size_t N>
    void assignStringToArray(std::array<char, N>& dest, std::string_view src) {
        static_assert(N > 0, "Array size must be greater than 0");
        const std::size_t length = std::min(src.size(), N - 1);

        std::ranges::copy_n(src.begin(), length, dest.begin());
        dest[length] = '\0';
    }
}