#pragma once

#include <nix/symbol-table.hh>

using nix::SymbolTable, nix::Symbol;

template <typename E> struct WithSymbols {
	E value;
	const SymbolTable& table;

	// fmap aka <$>
	template <typename F> constexpr inline auto map(F func) const;

	// $>
	template <typename B> constexpr auto replace(B value) const;

	constexpr inline const E* operator->() const;
	constexpr inline const E& operator*() const;
};

template <> struct WithSymbols<Symbol> {
	Symbol value;
	const SymbolTable& table;

	// fmap aka <$>
	template <typename F> constexpr inline auto map(F func) const;

	// $>
	template <typename B> constexpr auto replace(B value) const;

	constexpr inline const Symbol* operator->() const;
	constexpr inline const Symbol& operator*() const;

	inline std::optional<std::string_view> resolve() const {
		return this->value ? std::make_optional(this->table[this->value]) : std::nullopt;
	}

	constexpr inline operator std::string_view() const {
		return *this->resolve();
	}

	constexpr inline operator std::string() const {
		std::string_view str = *this;
		return std::string(str);
	}
};

template <typename E>
template <typename F>
constexpr inline auto WithSymbols<E>::map(F func) const {
	return WithSymbols<std::invoke_result_t<F, E>> {
		.value = func(this->value),
		.table = this->table,
	};
}

template <typename E>
template <typename B>
constexpr inline auto WithSymbols<E>::replace(B value) const {
	return this->map([value](auto) { return value; });
}

template <typename E> constexpr inline const E* WithSymbols<E>::operator->() const {
	return &this->value;
}

template <typename E> constexpr inline const E& WithSymbols<E>::operator*() const {
	return this->value;
}
