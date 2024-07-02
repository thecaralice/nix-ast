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

template <typename E>
std::void_t<decltype(to_json(std::declval<json&>(), std::declval<const E>()))>
	to_json(json& j, const WithSymbols<E>& expr);

template <typename T> void to_json(json& j, const WithSymbols<const T>& expr);

void to_json(json& j, const nix::ExprInt& expr);

void to_json(json& j, const nix::ExprFloat& expr);

void to_json(json& j, const nix::ExprString& expr);

void to_json(json& j, const nix::ExprPath& expr);

void to_json(json& j, const WithSymbols<Symbol>& sym);

void to_json(json& j, const WithSymbols<nix::ExprVar>& expr);

void to_json(json& j, const WithSymbols<nix::AttrName>& attr);

void to_json(json& j, const WithSymbols<nix::ExprAttrs::AttrDef>& expr);

void to_json(json& j, const WithSymbols<nix::ExprSelect>& expr);

void to_json(json& j, const WithSymbols<nix::ExprOpHasAttr>& expr);

void to_json(json& j, const WithSymbols<nix::ExprAttrs::DynamicAttrDef>& expr);

void to_json(json& j, const WithSymbols<nix::ExprAttrs>& expr);

void to_json(json& j, const WithSymbols<nix::ExprList>& expr);

void to_json(json& j, const WithSymbols<nix::Formal>& formal);

void to_json(json& j, const WithSymbols<nix::Formals>& formals);

void to_json(json& j, const WithSymbols<nix::ExprLambda>& expr);

void to_json(json& j, const WithSymbols<nix::ExprCall>& expr);

void to_json(json& j, const WithSymbols<nix::ExprLet>& expr);

void to_json(json& j, const WithSymbols<nix::ExprWith>& expr);

void to_json(json& j, const WithSymbols<nix::ExprIf>& expr);

void to_json(json& j, const WithSymbols<nix::ExprAssert>& expr);

void to_json(json& j, const WithSymbols<nix::ExprOpNot>& expr);

void to_json(json& j, const WithSymbols<nix::ExprConcatStrings>& expr);

void to_json(json& j, const WithSymbols<nix::ExprPos>& expr);

void to_json(json& j, const WithSymbols<nix::ExprOpEq>& expr);

void to_json(json& j, const WithSymbols<nix::ExprOpNEq>& expr);

void to_json(json& j, const WithSymbols<nix::ExprOpAnd>& expr);

void to_json(json& j, const WithSymbols<nix::ExprOpOr>& expr);

void to_json(json& j, const WithSymbols<nix::ExprOpImpl>& expr);

void to_json(json& j, const WithSymbols<nix::ExprOpUpdate>& expr);

void to_json(json& j, const WithSymbols<nix::ExprOpConcatLists>& expr);

template <typename E> void to_json(json& j, const WithSymbols<E*>& expr);

template <> void to_json(json& j, const WithSymbols<nix::Expr*>& expr);
