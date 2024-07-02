#pragma once

#include <nlohmann/json.hpp>
#include <ranges>
#include <type_traits>

#include "functor.hpp"

using json = nlohmann::json;

template <typename T> struct is_pair: std::false_type {};

template <typename A, typename B> struct is_pair<std::pair<A, B>>: std::true_type {};

template <typename T> constexpr bool is_pair_v = is_pair<T>::value;

template <std::ranges::range S>
std::enable_if_t<
	is_pair_v<std::ranges::range_value_t<S>>
		&& std::is_convertible_v<
			WithSymbols<std::remove_const_t<typename std::ranges::range_value_t<S>::first_type>>,
			json::object_t::key_type>,
	std::void_t<
		decltype(json(std::declval<WithSymbols<
										std::remove_const_t<typename std::ranges::range_value_t<S>::first_type>>>())),
		decltype(json(std::declval<WithSymbols<typename std::ranges::range_value_t<S>::second_type>>())
		)>>
	to_json(json& j, const WithSymbols<S>& expr) {
	auto trans = std::ranges::views::transform(*expr, [&expr](auto x) {
		return std::make_pair(std::string { expr.replace(x.first) }, expr.replace(x.second));
	});
	j = json::object_t(trans.begin(), trans.end());
}
