#pragma once

#include <type_traits>

template <typename T> struct is_pair: std::false_type {};

template <typename A, typename B> struct is_pair<std::pair<A, B>>: std::true_type {};

template <typename T> constexpr bool is_pair_v = is_pair<T>::value;

template <typename T>
concept pair = is_pair_v<T>;
