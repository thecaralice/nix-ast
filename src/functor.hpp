#pragma once

#include <nix/symbol-table.hh>
#include <nlohmann/json_fwd.hpp>
#include <ranges>
#include <tuple>
#include <utility>

using nix::SymbolTable, nix::Symbol;
using nlohmann::json;

template <typename E> struct WithSymbols {
	E value;
	const SymbolTable& table;

	// fmap aka <$>
	template <typename F> constexpr inline auto map(F func) const {
		return this->replace(func(this->value));
	}

	// $>
	template <typename B> constexpr inline auto replace(B value) const {
		return WithSymbols<B> { .value = value, .table = this->table };
	}

	constexpr inline const E* operator->() const {
		return &this->value;
	}

	constexpr inline const E& operator*() const {
		return this->value;
	}

	constexpr inline operator E() const {
		return this->value;
	}
};

template <typename E> struct WithSymbols<const E&> {
	const E& value;
	const SymbolTable& table;

	WithSymbols(const WithSymbols<E>& val): value(val.value), table(val.table) {}

	// fmap aka <$>
	template <typename F> constexpr inline auto map(F func) const {
		return this->replace(func(this->value));
	}

	// $>
	template <typename B> constexpr inline auto replace(B value) const {
		return WithSymbols<B> { .value = value, .table = this->table };
	}

	constexpr inline const E* operator->() const {
		return &this->value;
	}

	constexpr inline const E& operator*() const {
		return this->value;
	}

	constexpr inline operator E() const {
		return this->value;
	}

	constexpr inline operator E&() const {
		return this->value;
	}
};

template <> struct WithSymbols<Symbol> {
	Symbol value;
	const SymbolTable& table;

	inline std::optional<std::string_view> resolve() const {
		return this->value ? std::make_optional(this->table[this->value]) : std::nullopt;
	}

	inline operator std::string_view() const {
		return this->resolve().value();
	}

	inline operator std::string() const {
		std::string_view str = *this;
		return std::string(str);
	}
};

template <std::ranges::range S> constexpr inline auto lift_range(const WithSymbols<S>& range) {
	return std::ranges::views::transform(*range, [&range](auto&& x) { return range.replace(x); });
}

template <typename Tuple, std::size_t... Is>
constexpr inline auto _lift_tuple_impl(const WithSymbols<Tuple>& tup, std::index_sequence<Is...>) {
	return std::make_tuple(tup.map([](auto&& x) { return x.template get<Is>(x); })...);
}

template <typename... Ts>
constexpr inline auto lift_tuple(const WithSymbols<std::tuple<Ts...>>& tup) {
	return _lift_tuple_impl(tup, std::index_sequence_for<Ts...> {});
}

template <typename L, typename R>
constexpr inline auto lift_pair(const WithSymbols<std::pair<L, R>>& pair) {
	return std::pair {
		pair.map([](auto&& x) -> L { return x.first; }),
		pair.map([](auto&& x) -> R { return x.second; }),
	};
}
