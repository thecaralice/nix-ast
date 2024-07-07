#pragma once

#include <nix/nixexpr.hh>

#include "functor.hpp"
#include "rtti.hpp"

using json = nlohmann::json;

using expr_variant = rwcv<
	nix::ExprInt,
	nix::ExprFloat,
	nix::ExprString,
	nix::ExprPath,
	nix::ExprVar,
	nix::ExprSelect,
	nix::ExprOpHasAttr,
	nix::ExprAttrs,
	nix::ExprList,
	nix::ExprLambda,
	nix::ExprCall,
	nix::ExprLet,
	nix::ExprWith,
	nix::ExprIf,
	nix::ExprAssert,
	nix::ExprOpNot,
	nix::ExprConcatStrings,
	nix::ExprPos,
	nix::ExprOpEq,
	nix::ExprOpNEq,
	nix::ExprOpAnd,
	nix::ExprOpOr,
	nix::ExprOpImpl,
	nix::ExprOpUpdate,
	nix::ExprOpConcatLists>;

void to_json(json& j, const nix::ExprInt& expr);

void to_json(json& j, const nix::ExprFloat& expr);

void to_json(json& j, const nix::ExprString& expr);

void to_json(json& j, const nix::ExprPath& expr);

void to_json(json& j, const WithSymbols<Symbol>& sym);

void to_json(json& j, const WithSymbols<const nix::ExprVar&>& expr);

void to_json(json& j, const WithSymbols<const nix::AttrName&>& attr);

void to_json(json& j, const WithSymbols<const nix::ExprAttrs::AttrDef&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprSelect&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprOpHasAttr&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprAttrs::DynamicAttrDef&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprAttrs&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprList&>& expr);

void to_json(json& j, const WithSymbols<const nix::Formal&>& formal);

void to_json(json& j, const WithSymbols<const nix::Formals&>& formals);

void to_json(json& j, const WithSymbols<const nix::ExprLambda&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprCall&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprLet&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprWith&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprIf&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprAssert&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprOpNot&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprConcatStrings&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprPos&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprOpEq&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprOpNEq&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprOpAnd&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprOpOr&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprOpImpl&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprOpUpdate&>& expr);

void to_json(json& j, const WithSymbols<const nix::ExprOpConcatLists&>& expr);

template <typename E> void to_json(json& j, const WithSymbols<E*>& expr);

template <std::ranges::range S> void to_json(json& j, const WithSymbols<S>& range);
