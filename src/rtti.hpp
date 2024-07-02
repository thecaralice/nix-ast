#pragma once

#include <functional>
#include <optional>
#include <sstream>
#include <variant>

template <typename V, typename P> auto dyn_to_variant(const P* dyn) -> V {
	return [&]<typename... Ts>(std::type_identity<std::variant<std::reference_wrapper<Ts>...>>) {
		std::optional<V> res;
		((dynamic_cast<const Ts*>(dyn)
				? (res = std::reference_wrapper<Ts> { *static_cast<const Ts*>(dyn) }, 0)
				: 0),
		 ...);
		if (res) {
			return *res;
		}
		std::stringstream err;
		err << "Unable to read pointer " << dyn << "(typeid: " << typeid(*dyn).name() << ")";
		throw std::runtime_error(err.str());
	}(std::type_identity<V> {});
}

template <typename... V> using rwcv = std::variant<std::reference_wrapper<const V>...>;
