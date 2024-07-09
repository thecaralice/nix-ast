#include <nix/nixexpr.hh>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <utility>

#include "expr_json.hpp"
#include "functor.hpp"
#include "is_pair.hpp"
#include "rtti.hpp"

using json = nlohmann::json;

void to_json(json& j, const nix::ExprInt& expr) {
	j = {
		{ "kind", "Int" },
#if NIX_VERSION_MINOR < 22
		{ "value", expr.v.integer },
#else
		{ "value", expr.v.integer() },
#endif
	};
}

void to_json(json& j, const nix::ExprFloat& expr) {
	j = {
		{ "kind", "Float" },
#if NIX_VERSION_MINOR < 22
		{ "value", expr.v.fpoint },
#else
		{ "value", expr.v.fpoint() },
#endif
	};
}

void to_json(json& j, const nix::ExprString& expr) {
	j = {
		{ "kind", "String" },
		{ "value", expr.s },
	};
}

void to_json(json& j, const nix::ExprPath& expr) {
	j = {
		{ "kind", "Path" },
		{ "value", expr.s },
	};
}

void to_json(json& j, const WithSymbols<Symbol>& sym) {
	if (auto s = sym.resolve()) {
		std::string_view str = *s;
		j = str;
	} else {
		j = nullptr;
	}
}

void to_json(json& j, const WithSymbols<const nix::ExprVar&>& expr) {
	j = {
		{ "kind", "Var" },
		{ "name", expr.map([](auto&& x) { return x.name; }) },
#if 18 <= NIX_VERSION_MINOR && NIX_VERSION_MINOR < 20
		{ "from_with", expr->fromWith },
#else
		{ "from_with", expr.map([](auto&& x) { return x.fromWith; }) },
#endif
		{ "level", expr->level },
		{ "displacement", expr->displ },
	};
}

void to_json(json& j, const WithSymbols<const nix::AttrName&>& attr) {
	if (attr->symbol) {
		j = attr.map([](auto&& x) { return x.symbol; });
	} else {
		j = attr.map([](auto&& x) { return x.expr; });
	}
}

void to_json(json& j, const WithSymbols<const nix::ExprSelect&>& expr) {
	j = {
		{ "kind", "Select" },
		{ "from", expr.map([](auto&& x) { return x.e; }) },
		{ "path", expr.map([](auto&& x) { return x.attrPath; }) },
		{ "default", expr.map([](auto&& x) { return x.def; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprOpHasAttr&>& expr) {
	j = {
		{ "kind", "OpHasAttr" },
		{ "from", expr.map([](auto&& x) { return x.e; }) },
		{ "path", expr.map([](auto&& x) { return x.attrPath; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprAttrs::DynamicAttrDef&>& expr) {
	j = {
		{ "name", expr.map([](auto&& x) { return x.nameExpr; }) },
		{ "value", expr.map([](auto&& x) { return x.valueExpr; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprAttrs&>& expr) {
	auto attrs = expr.map([](auto&& x) { return x.attrs; });
#if NIX_VERSION_MINOR < 21
	std::map<WithSymbols<nix::Symbol>, WithSymbols<nix::Expr*>> plain;
	std::set<WithSymbols<nix::Symbol>> inherited;
	for (auto&& x: lift_range(attrs)) {
		auto&& [name, attr] = lift_pair(x);
		if (attr->inherited) {
			inherited.emplace(name);
		} else {
			plain.emplace(name, attr.map([](auto&& y) { return y.e; }));
		}
	}
	json attrs_json = {
		{ "plain", plain },
		{ "inherited", inherited },
		{ "dynamic", expr.map([](auto&& x) { return x.dynamicAttrs; }) },
	};
#else
	std::map<WithSymbols<nix::Symbol>, WithSymbols<nix::Expr*>> plain;
	std::set<WithSymbols<nix::Symbol>> inherited;
	std::map<nix::ExprInheritFrom*, std::set<WithSymbols<nix::Symbol>>> inherited_from;
	for (auto&& x: lift_range(attrs)) {
		auto&& [name, attr] = lift_pair(x);
		using Kind = nix::ExprAttrs::AttrDef::Kind;
		switch (attr->kind) {
		case Kind::Plain: plain.emplace(name, attr.map([](auto&& y) { return y.e; })); break;
		case Kind::Inherited: inherited.emplace(name); break;
		case Kind::InheritedFrom:
			if (!expr->inheritFromExprs) {
				throw std::runtime_error("got a Kind::InheritedFrom, but inheritFromExprs is empty");
			}
			inherited_from
				.try_emplace(dynamic_cast<nix::ExprInheritFrom*>(dynamic_cast<nix::ExprSelect*>(attr->e)->e)
				)
				.first->second.emplace(name);
			break;
		}
	}
	auto inherited_from_view
		= std::ranges::views::transform(inherited_from, [&expr](auto&& p) -> json {
				return {
					{ "from", expr.map([&p](auto&& x) { return (*x.inheritFromExprs)[p.first->displ]; }) },
					{ "attrs", p.second },
				};
			});
	json attrs_json = {
		{ "plain", plain },
		{ "inherited", inherited },
		{ "inherited_from", json::array_t(inherited_from_view.begin(), inherited_from_view.end()) },
		{ "dynamic", expr.map([](auto&& x) { return x.dynamicAttrs; }) },
	};
#endif
	j = {
		{ "kind", "Attrs" },
		{ "recursive", expr->recursive },
		{ "attrs", attrs_json },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprList&>& expr) {
	j = {
		{ "kind", "List" },
		{ "value", expr.map([](auto&& x) { return x.elems; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::Formal&>& formal) {
	j = {
		{ "name", formal.map([](auto&& x) { return x.name; }) },
		{ "default", formal.map([](auto&& x) { return x.def; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::Formals&>& formals) {
	j = {
		{ "ellipsis", formals->ellipsis },
		{ "formals", formals.map([](auto&& x) { return x.formals; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprLambda&>& expr) {
	j = {
		{ "kind", "Lambda" },
		{ "name", expr.map([](auto&& x) { return x.name; }) },
		{ "arg", expr.map([](auto&& x) { return x.arg; }) },
		{ "body", expr.map([](auto&& x) { return x.body; }) },
		{ "formals", expr.map([](auto&& x) { return x.formals; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprCall&>& expr) {
	j = {
		{ "kind", "Call" },
		{ "function", expr.map([](auto&& x) { return x.fun; }) },
		{ "args", expr.map([](auto&& x) { return x.args; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprLet&>& expr) {
	j = {
		{ "kind", "Let" },
		{ "attrs", expr.map([](auto&& x) { return x.attrs; }) },
		{ "body", expr.map([](auto&& x) { return x.body; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprWith&>& expr) {
	j = {
		{ "kind", "With" },
		{ "attrs", expr.map([](auto&& x) { return x.attrs; }) },
		{ "body", expr.map([](auto&& x) { return x.body; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprIf&>& expr) {
	j = {
		{ "kind", "If" },
		{ "cond", expr.map([](auto&& x) { return x.cond; }) },
		{ "then", expr.map([](auto&& x) { return x.then; }) },
		{ "else", expr.map([](auto&& x) { return x.else_; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprAssert&>& expr) {
	j = {
		{ "kind", "Assert" },
		{ "cond", expr.map([](auto&& x) { return x.cond; }) },
		{ "body", expr.map([](auto&& x) { return x.body; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprOpNot&>& expr) {
	j = {
		{ "kind", "OpNot" },
		{ "body", expr.map([](auto&& x) { return x.e; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprConcatStrings&>& expr) {
	j = {
		{ "kind", "ConcatStrings" },
		{ "forceString", expr->forceString },
		{
			"strings",
			expr.map([](auto x) {
				return std::ranges::views::transform(*x.es, [](auto&& p) -> nix::Expr* {
					return p.second;
				});
			}),
		},
	};
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

void to_json(json& j, const WithSymbols<const nix::ExprPos&>& _expr) {
	j = {
		{ "kind", "Pos" },
	};
}

#pragma clang diagnostic pop

void to_json(json& j, const WithSymbols<const nix::ExprOpEq&>& expr) {
	j = {
		{ "kind", "OpEq" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprOpNEq&>& expr) {
	j = {
		{ "kind", "OpNEq" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprOpAnd&>& expr) {
	j = {
		{ "kind", "OpAnd" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprOpOr&>& expr) {
	j = {
		{ "kind", "OpOr" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprOpImpl&>& expr) {
	j = {
		{ "kind", "OpImpl" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprOpUpdate&>& expr) {
	j = {
		{ "kind", "OpUpdate" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

void to_json(json& j, const WithSymbols<const nix::ExprOpConcatLists&>& expr) {
	j = {
		{ "kind", "OpConcatLists" },
		{ "left", expr.map([](auto&& x) { return x.e1; }) },
		{ "right", expr.map([](auto&& x) { return x.e2; }) },
	};
}

template <typename E> void to_json(json& j, const WithSymbols<E*>& expr) {
	if (!*expr) {
		return;
	}
	// j = expr.map([](auto&& x) -> E& { return *x; });
	j = expr.template replace<const E&>(**expr);
}

template <typename... V> void to_json(json& j, const WithSymbols<rwcv<V...>>& expr) {
	std::visit(
		[&j, &expr](auto& x) {
			using T = std::decay_t<decltype(x)>;
			using U = std::remove_const_t<std::remove_reference_t<typename T::type>>;
			static_assert(std::is_same_v<T, std::reference_wrapper<const U>>);
			WithSymbols<const U&> r(x, expr.table);
			to_json(j, r);
		},
		*expr
	);
}

template <> void to_json(json& j, const WithSymbols<nix::Expr*>& expr) {
	if (!*expr) {
		j = nullptr;
		return;
	}
	j = expr.map([](nix::Expr* ptr) { return dyn_to_variant<expr_variant>(ptr); });
}

template <std::ranges::range S>
	requires(!pair<typename std::ranges::range_value_t<S>>)
void to_json(json& j, const WithSymbols<S>& range) {
	auto t = lift_range(range);
	j = json::array_t(t.begin(), t.end());
}

template <std::ranges::range S>
	requires(pair<typename std::ranges::range_value_t<S>>)
void to_json(json& j, const WithSymbols<S>& range) {
	auto t = std::ranges::views::transform(lift_range(range), [](auto&& x) {
		auto p = lift_pair(x);
		return std::make_pair(std::string_view { p.first }, p.second);
	});
	j = json::object_t(t.begin(), t.end());
}
