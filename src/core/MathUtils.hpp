#pragma once

#include <algorithm>
#include <cmath>

namespace solar::core {

template <typename T>
[[nodiscard]] constexpr T clamp(const T value, const T minimum, const T maximum)
{
    return std::clamp(value, minimum, maximum);
}

template <typename T>
[[nodiscard]] constexpr T radians(const T degrees)
{
    return degrees * static_cast<T>(0.01745329251994329576923690768489L);
}

} // namespace solar::core

