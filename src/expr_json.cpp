#include <nix/nixexpr.hh>
#include <nlohmann/json.hpp>

#include "expr_json.hpp"
#include "functor.hpp"
#include "is_pair.hpp"
#include "rtti.hpp"

using json = nlohmann::json;

template <typename T> void to_json(json& j, const WithSymbols<const T>& expr) {
	return to_json(j, expr.map([](auto&& x) -> WithSymbols<T> { return x; }));
}

void to_json(json& j, const nix::ExprInt& expr) {
	j = json {
		{ "kind", "Int" },
		{ "value", expr.v.integer },
	};
}

void to_json(json& j, const nix::ExprFloat& expr) {
	j = json {
		{ "kind", "Float" },
		{ "value", expr.v.fpoint },
	};
}

void to_json(json& j, const nix::ExprString& expr) {
	j = json {
		{ "kind", "String" },
		{ "value", expr.s },
	};
}

void to_json(json& j, const nix::ExprPath& expr) {
	j = json {
		{ "kind", "Path" },
		{ "value", expr.s },
	};
}

void to_json(json& j, const WithSymbols<Symbol>& sym) {
	if (auto s = sym.resolve()) {
		std::string_view str = *s;
		j = str;
	} else {
		j = json();
	}
}

template <std::ranges::range S>
std::enable_if_t<
	!is_pair_v<std::ranges::range_value_t<S>>,
	std::void_t<decltype(json(std::declval<WithSymbols<std::ranges::range_value_t<S>>>()))>>
	to_json(json& j, const WithSymbols<S>& expr) {
	auto trans = std::ranges::views::transform(*expr, [&expr](auto&& x) { return expr.replace(x); });
	j = json::array_t(trans.begin(), trans.end());
}

void to_json(json& j, const WithSymbols<nix::ExprVar>& expr) {
	j = json {
		{ "kind", "Var" },
		{ "name", expr.map([](auto x) { return x.name; }) },
		{ "from_with", expr.map([](auto&& x) { return x.fromWith; }) },
		{ "level", expr->level },
		{ "displacement", expr->displ },
	};
}

void to_json(json& j, const WithSymbols<nix::AttrName>& attr) {
	if (attr->symbol) {
		j = attr.map([](auto&& x) { return x.symbol; });
	} else {
		j = attr.map([](auto&& x) { return x.expr; });
	}
}

void to_json(json& j, const WithSymbols<nix::ExprAttrs::AttrDef>& expr) {
	j = {
		{ "inherited", expr->inherited },
		{ "value", expr.map([](auto&& x) { return x.e; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprSelect>& expr) {
	j = json {
		{ "kind", "Select" },
		{ "from", expr.map([](auto&& x) { return x.e; }) },
		{ "path", expr.map([](auto&& x) { return x.attrPath; }) },
		{ "default", expr.map([](auto&& x) { return x.def; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprOpHasAttr>& expr) {
	j = json {
		{ "kind", "OpHasAttr" },
		{ "from", expr.map([](auto&& x) { return x.e; }) },
		{ "path", expr.map([](auto&& x) { return x.attrPath; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprAttrs::DynamicAttrDef>& expr) {
	j = {
		{ "name", expr.map([](auto&& x) { return x.nameExpr; }) },
		{ "value", expr.map([](auto&& x) { return x.valueExpr; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprAttrs>& expr) {
	j = json {
		{ "kind", "Attrs" },
		{ "recursive", expr->recursive },
		{ "attrs", expr.map([](auto&& x) { return x.attrs; }) },
		{ "dynamic", expr.map([](auto&& x) { return x.dynamicAttrs; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprList>& expr) {
	j = {
		{ "kind", "List" },
		{ "value", expr.map([](auto&& x) { return x.elems; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::Formal>& formal) {
	j = {
		{ "name", formal.map([](auto&& x) { return x.name; }) },
		{ "default", formal.map([](auto&& x) { return x.def; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::Formals>& formals) {
	j = {
		{ "ellipsis", formals->ellipsis },
		{ "formals", formals.map([](auto&& x) { return x.formals; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprLambda>& expr) {
	j = {
		{ "kind", "Lambda" },
		{ "name", expr.map([](auto&& x) { return x.name; }) },
		{ "arg", expr.map([](auto&& x) { return x.arg; }) },
		{ "body", expr.map([](auto&& x) { return x.body; }) },
		{ "formals", expr.map([](auto&& x) { return x.formals; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprCall>& expr) {
	j = {
		{ "kind", "Call" },
		{ "function", expr.map([](auto&& x) { return x.fun; }) },
		{ "args", expr.map([](auto&& x) { return x.args; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprLet>& expr) {
	j = {
		{ "kind", "Let" },
		{ "attrs", expr.map([](auto&& x) { return *x.attrs; }) },
		{ "body", expr.map([](auto&& x) { return x.body; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprWith>& expr) {
	j = {
		{ "kind", "With" },
		{ "attrs", expr.map([](auto&& x) { return x.attrs; }) },
		{ "body", expr.map([](auto&& x) { return x.body; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprIf>& expr) {
	j = {
		{ "kind", "If" },
		{ "cond", expr.map([](auto&& x) { return x.cond; }) },
		{ "then", expr.map([](auto&& x) { return x.then; }) },
		{ "else", expr.map([](auto&& x) { return x.else_; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprAssert>& expr) {
	j = {
		{ "kind", "Assert" },
		{ "cond", expr.map([](auto&& x) { return x.cond; }) },
		{ "body", expr.map([](auto&& x) { return x.body; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprOpNot>& expr) {
	j = {
		{ "kind", "OpNot" },
		{ "body", expr.map([](auto&& x) { return x.e; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprConcatStrings>& expr) {
	j = {
		{ "kind", "ConcatStrings" },
		{ "forceString", expr->forceString },
		{ "strings", expr.map([](auto x) {
			 return std::ranges::views::transform(*x.es, [](auto&& p) -> nix::Expr* { return p.second; });
		 }) },
	};
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

void to_json(json& j, const WithSymbols<nix::ExprPos>& _expr) {
	j = {
		{ "kind", "Pos" },
	};
}

#pragma clang diagnostic pop

void to_json(json& j, const WithSymbols<nix::ExprOpEq>& expr) {
	j = {
		{ "kind", "OpEq" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprOpNEq>& expr) {
	j = {
		{ "kind", "OpNEq" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprOpAnd>& expr) {
	j = {
		{ "kind", "OpAnd" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprOpOr>& expr) {
	j = {
		{ "kind", "OpOr" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprOpImpl>& expr) {
	j = {
		{ "kind", "OpImpl" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprOpUpdate>& expr) {
	j = {
		{ "kind", "OpUpdate" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<nix::ExprOpConcatLists>& expr) {
	j = {
		{ "kind", "OpConcatLists" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

template <typename E>
std::void_t<decltype(to_json(std::declval<json&>(), std::declval<const E>()))>
	to_json(json& j, const WithSymbols<E>& expr) {
	return to_json(j, expr.value);
}

template <typename E> void to_json(json& j, const WithSymbols<E*>& expr) {
	if (!*expr) {
		return;
	}
	j = expr.map([](auto&& x) { return *x; });
}

template <typename... V> void to_json(json& j, const WithSymbols<rwcv<V...>>& expr) {
	auto& [value, table] = expr;
	std::visit(
		[&table, &j](auto&& x) {
			using T = std::decay_t<decltype(x)>;
			using U = std::remove_const_t<std::remove_reference_t<typename T::type>>;
			static_assert(std::is_same_v<T, std::reference_wrapper<const U>>);
			WithSymbols<U> r {
				.value = x,
				.table = table,
			};
			return to_json(j, r);
		},
		value
	);
}

template <> void to_json(json& j, const WithSymbols<nix::Expr*>& expr) {
	if (!*expr) {
		return;
	}
	WithSymbols<expr_variant> v
		= expr.map([](nix::Expr* ptr) { return dyn_to_variant<expr_variant>(ptr); });
	return to_json(j, v);
}
