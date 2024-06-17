#include <nix/config.h>

#include <nix/command.hh>
#include <nix/eval-settings.hh>
#include <nix/eval.hh>

using namespace nix;
using json = nlohmann::json;

template <typename V, typename P> auto dyn_to_variant(const P *dyn) -> V {
  return [&]<typename... Ts>(
             std::type_identity<std::variant<std::reference_wrapper<Ts>...>>) {
    std::optional<V> res;
    ((dynamic_cast<const Ts *>(dyn)
          ? (res = std::reference_wrapper<Ts>{*static_cast<const Ts *>(dyn)}, 0)
          : 0),
     ...);
    if (res) {
      return *res;
    }
    std::stringstream err;
    err << "Unable to read pointer " << dyn
        << "(typeid: " << typeid(*dyn).name() << ")";
    throw std::runtime_error(err.str());
  }(std::type_identity<V>{});
};
using expr_variant =
    std::variant<std::reference_wrapper<const ExprInt>,
                 std::reference_wrapper<const ExprFloat>,
                 std::reference_wrapper<const ExprString>,
                 std::reference_wrapper<const ExprPath>,
                 std::reference_wrapper<const ExprVar>,
                 std::reference_wrapper<const ExprSelect>,
                 std::reference_wrapper<const ExprOpHasAttr>,
                 std::reference_wrapper<const ExprAttrs>,
                 std::reference_wrapper<const ExprList>,
                 std::reference_wrapper<const ExprLambda>,
                 std::reference_wrapper<const ExprCall>,
                 std::reference_wrapper<const ExprLet>,
                 std::reference_wrapper<const ExprWith>,
                 std::reference_wrapper<const ExprIf>,
                 std::reference_wrapper<const ExprAssert>,
                 std::reference_wrapper<const ExprOpNot>,
                 std::reference_wrapper<const ExprConcatStrings>,
                 std::reference_wrapper<const ExprPos>,
                 std::reference_wrapper<const ExprOpEq>,
                 std::reference_wrapper<const ExprOpNEq>,
                 std::reference_wrapper<const ExprOpAnd>,
                 std::reference_wrapper<const ExprOpOr>,
                 std::reference_wrapper<const ExprOpImpl>,
                 std::reference_wrapper<const ExprOpUpdate>,
                 std::reference_wrapper<const ExprOpConcatLists>>;

template <typename E> struct WithSymbols {
  E value;
  const SymbolTable &table;

  // fmap aka <$>
  template <typename F> constexpr inline auto map(F func) const;

  // $>
  template <typename B> constexpr auto replace(B value) const;

  constexpr inline const E *operator->() const;
  constexpr inline const E &operator*() const;
};

template <> struct WithSymbols<Symbol> {
  Symbol value;
  const SymbolTable &table;

  // fmap aka <$>
  template <typename F> constexpr inline auto map(F func) const;

  // $>
  template <typename B> constexpr auto replace(B value) const;

  constexpr inline const Symbol *operator->() const;
  constexpr inline const Symbol &operator*() const;
  inline std::optional<std::string_view> resolve() const {
    return this->value ? std::make_optional(this->table[this->value])
                       : std::nullopt;
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
  return WithSymbols<std::invoke_result_t<F, E>>{
      .value = func(this->value),
      .table = this->table,
  };
}

template <typename E>
template <typename B>
constexpr inline auto WithSymbols<E>::replace(B value) const {
  return this->map([value](auto) { return value; });
}
template <typename E>
constexpr inline const E *WithSymbols<E>::operator->() const {
  return &this->value;
}

template <typename E>
constexpr inline const E &WithSymbols<E>::operator*() const {
  return this->value;
}

template <typename E> void to_json(json &j, const WithSymbols<E *> &expr);

template <typename T> void to_json(json &j, const WithSymbols<const T> &expr) {
  return to_json(j, expr.map([](auto &&x) -> WithSymbols<T> { return x; }));
}

void to_json(json &j, const ExprInt &expr) {
  j = json{
      {"kind", "Int"},
      {"value", expr.v.integer},
  };
}
void to_json(json &j, const ExprFloat &expr) {
  j = json{
      {"kind", "Float"},
      {"value", expr.v.fpoint},
  };
}
void to_json(json &j, const ExprString &expr) {
  j = json{
      {"kind", "String"},
      {"value", expr.s},
  };
}
void to_json(json &j, const ExprPath &expr) {
  j = json{
      {"kind", "Path"},
      {"value", expr.s},
  };
}
void to_json(json &j, const WithSymbols<Symbol> &sym) {
  if (auto s = sym.resolve()) {
    std::string_view str = *s;
    j = str;
  } else {
    j = json();
  }
}

template <typename T> struct is_pair : std::false_type {};

template <typename A, typename B>
struct is_pair<std::pair<A, B>> : std::true_type {};

template <typename T> constexpr bool is_pair_v = is_pair<T>::value;

template <std::ranges::range S>
std::enable_if_t<
    is_pair_v<std::ranges::range_value_t<S>> &&
        std::is_convertible_v<
            WithSymbols<std::remove_const_t<
                typename std::ranges::range_value_t<S>::first_type>>,
            json::object_t::key_type>,
    std::void_t<
        decltype(json(
            std::declval<WithSymbols<std::remove_const_t<
                typename std::ranges::range_value_t<S>::first_type>>>())),
        decltype(json(
            std::declval<WithSymbols<
                typename std::ranges::range_value_t<S>::second_type>>()))>>
to_json(json &j, const WithSymbols<S> &expr) {
  auto trans = std::ranges::views::transform(*expr, [&expr](auto x) {
    return std::make_pair(std::string{expr.replace(x.first)},
                          expr.replace(x.second));
  });
  j = json::object_t(trans.begin(), trans.end());
}

template <std::ranges::range S>
std::enable_if_t<
    !is_pair_v<std::ranges::range_value_t<S>>,
    std::void_t<decltype(json(
        std::declval<WithSymbols<std::ranges::range_value_t<S>>>()))>>
to_json(json &j, const WithSymbols<S> &expr) {
  auto trans = std::ranges::views::transform(
      *expr, [&expr](auto &&x) { return expr.replace(x); });
  j = json::array_t(trans.begin(), trans.end());
}

void to_json(json &j, const WithSymbols<ExprVar> &expr) {
  j = json{
      {"kind", "Var"},
      {"name", expr.map([](auto x) { return x.name; })},
      {"from_with", expr.map([](auto &&x) { return x.fromWith; })},
      {"level", expr->level},
      {"displacement", expr->displ},
  };
}

void to_json(json &j, const WithSymbols<AttrName> &attr) {
  if (attr->symbol) {
    j = attr.map([](auto &&x) { return x.symbol; });
  } else {
    j = attr.map([](auto &&x) { return x.expr; });
  }
}

void to_json(json &j, const WithSymbols<ExprAttrs::AttrDef> &expr) {
  j = {
      {"inherited", expr->inherited},
      {"value", expr.map([](auto &&x) { return x.e; })},
  };
}

void to_json(json &j, const WithSymbols<ExprSelect> &expr) {
  j = json{{"kind", "Select"},
           {"from", expr.map([](auto &&x) { return x.e; })},
           {"path", expr.map([](auto &&x) { return x.attrPath; })},
           {"default", expr.map([](auto &&x) { return x.def; })}};
}

void to_json(json &j, const WithSymbols<ExprOpHasAttr> &expr) {
  j = json{
      {"kind", "OpHasAttr"},
      {"from", expr.map([](auto &&x) { return x.e; })},
      {"path", expr.map([](auto &&x) { return x.attrPath; })},
  };
}

void to_json(json &j, const WithSymbols<ExprAttrs::DynamicAttrDef> &expr) {
  j = {
      {"name", expr.map([](auto &&x) { return x.nameExpr; })},
      {"value", expr.map([](auto &&x) { return x.valueExpr; })},
  };
}

void to_json(json &j, const WithSymbols<ExprAttrs> &expr) {
  j = json{
      {"kind", "Attrs"},
      {"recursive", expr->recursive},
      {"attrs", expr.map([](auto &&x) { return x.attrs; })},
      {"dynamic", expr.map([](auto &&x) { return x.dynamicAttrs; })},

  };
}
void to_json(json &j, const WithSymbols<ExprList> &expr) {
  j = {
      {"kind", "List"},
      {"value", expr.map([](auto &&x) { return x.elems; })},
  };
}

void to_json(json &j, const WithSymbols<Formal> &formal) {
  j = {
      {"name", formal.map([](auto &&x) { return x.name; })},
      {"default", formal.map([](auto &&x) { return x.def; })},
  };
}

void to_json(json &j, const WithSymbols<Formals> &formals) {
  j = {
      {"ellipsis", formals->ellipsis},
      {"formals", formals.map([](auto &&x) { return x.formals; })},
  };
}

void to_json(json &j, const WithSymbols<ExprLambda> &expr) {
  j = {
      {"kind", "Lambda"},
      {"name", expr.map([](auto &&x) { return x.name; })},
      {"arg", expr.map([](auto &&x) { return x.arg; })},
      {"body", expr.map([](auto &&x) { return x.body; })},
      {"formals", expr.map([](auto &&x) { return x.formals; })},
  };
}
void to_json(json &j, const WithSymbols<ExprCall> &expr) {
  j = {
      {"kind", "Call"},
      {"function", expr.map([](auto &&x) { return x.fun; })},
      {"args", expr.map([](auto &&x) { return x.args; })},
  };
}
void to_json(json &j, const WithSymbols<ExprLet> &expr) {
  j = {
      {"kind", "Let"},
      {"attrs", expr.map([](auto &&x) { return *x.attrs; })},
      {"body", expr.map([](auto &&x) { return x.body; })},
  };
}
void to_json(json &j, const WithSymbols<ExprWith> &expr) {
  j = {
      {"kind", "With"},
      {"attrs", expr.map([](auto &&x) { return x.attrs; })},
      {"body", expr.map([](auto &&x) { return x.body; })},
  };
}
void to_json(json &j, const WithSymbols<ExprIf> &expr) {
  j = {
      {"kind", "If"},
      {"cond", expr.map([](auto &&x) { return x.cond; })},
      {"then", expr.map([](auto &&x) { return x.then; })},
      {"else", expr.map([](auto &&x) { return x.else_; })},
  };
}
void to_json(json &j, const WithSymbols<ExprAssert> &expr) {
  j = {
      {"kind", "Assert"},
      {"cond", expr.map([](auto &&x) { return x.cond; })},
      {"body", expr.map([](auto &&x) { return x.body; })},
  };
}
void to_json(json &j, const WithSymbols<ExprOpNot> &expr) {
  j = {
      {"kind", "OpNot"},
      {"body", expr.map([](auto &&x) { return x.e; })},
  };
}
void to_json(json &j, const WithSymbols<ExprConcatStrings> &expr) {
  j = {
      {"kind", "ConcatStrings"},
      {"forceString", expr->forceString},
      {"strings", expr.map([](auto x) {
         return std::ranges::views::transform(
             *x.es, [](auto &&p) -> Expr * { return p.second; });
       })},
  };
}
void to_json(json &j, const WithSymbols<ExprPos> &expr) {
  j = {
      {"kind", "Pos"},
  };
}
void to_json(json &j, const WithSymbols<ExprOpEq> &expr) {
  j = {
      {"kind", "OpEq"},
      {"left", expr.map([](auto &&x) { return x.e1; })},
      {"right", expr.map([](auto &&x) { return x.e2; })},
  };
}
void to_json(json &j, const WithSymbols<ExprOpNEq> &expr) {
  j = {
      {"kind", "OpNEq"},
      {"left", expr.map([](auto &&x) { return x.e1; })},
      {"right", expr.map([](auto &&x) { return x.e2; })},
  };
}
void to_json(json &j, const WithSymbols<ExprOpAnd> &expr) {
  j = {
      {"kind", "OpAnd"},
      {"left", expr.map([](auto &&x) { return x.e1; })},
      {"right", expr.map([](auto &&x) { return x.e2; })},
  };
}
void to_json(json &j, const WithSymbols<ExprOpOr> &expr) {
  j = {
      {"kind", "OpOr"},
      {"left", expr.map([](auto &&x) { return x.e1; })},
      {"right", expr.map([](auto &&x) { return x.e2; })},
  };
}
void to_json(json &j, const WithSymbols<ExprOpImpl> &expr) {
  j = {
      {"kind", "OpImpl"},
      {"left", expr.map([](auto &&x) { return x.e1; })},
      {"right", expr.map([](auto &&x) { return x.e2; })},
  };
}
void to_json(json &j, const WithSymbols<ExprOpUpdate> &expr) {
  j = {
      {"kind", "OpUpdate"},
      {"left", expr.map([](auto &&x) { return x.e1; })},
      {"right", expr.map([](auto &&x) { return x.e2; })},
  };
}
void to_json(json &j, const WithSymbols<ExprOpConcatLists> &expr) {
  j = {
      {"kind", "OpConcatLists"},
      {"left", expr.map([](auto &&x) { return x.e1; })},
      {"right", expr.map([](auto &&x) { return x.e2; })},
  };
}

template <typename E>
std::void_t<decltype(to_json(std::declval<json &>(), std::declval<const E>()))>
to_json(json &j, const WithSymbols<E> &expr) {
  return to_json(j, expr.value);
}

template <typename E> void to_json(json &j, const WithSymbols<E *> &expr) {
  if (!*expr) {
    return;
  }
  j = expr.map([](auto &&x) { return *x; });
}

template <> void to_json(json &j, const WithSymbols<Expr *> &expr) {
  if (!*expr) {
    return;
  }
  std::visit(
      [&expr, &j](auto &&x) {
        using T = std::decay_t<decltype(x)>;
        using U =
            std::remove_const_t<std::remove_reference_t<typename T::type>>;
        static_assert(std::is_same_v<T, std::reference_wrapper<const U>>);
        WithSymbols<U> r = expr.replace(x.get());
        return to_json(j, r);
      },
      dyn_to_variant<expr_variant>(expr.value));
}

struct CmdAst : SourceExprCommand {
  CmdAst() {}
  Category category() override { return catUtility; }
  std::string description() override {
    return "show a JSON representation of a Nix expression";
  }
  std::string doc() override {
    return
#include "ast.md"
        ;
  }
  void run(ref<Store>) override {
    evalSettings.pureEval = false;
    auto eval_state = getEvalState();
    if (this->expr && this->file) {
      throw UsageError("`--file` and `--expr` are mutually exclusive");
    }
    if (!this->file && !this->expr) {
      this->file = "-";
    }
    Expr *expr;
    if (this->file) {
      expr = eval_state->parseExprFromFile(
          eval_state->rootPath(CanonPath::fromCwd(this->file.value())));
    } else {
      expr = eval_state->parseExprFromString(
          this->expr.value(),
          eval_state->rootPath(CanonPath::fromCwd(getCommandBaseDir())));
    }
    json j = WithSymbols<Expr *>{.value = expr, .table = eval_state->symbols};
    std::cout << j << std::endl;
  }
};
static auto rCmdAst = registerCommand<CmdAst>("ast");
