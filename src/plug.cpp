#include <nix/config.h>

#include <nix/command.hh>
#include <nix/eval-settings.hh>
#include <nix/eval.hh>

#include "expr_json.hpp"
#include "functor.hpp"
#include "polyfill.hpp"

using json = nlohmann::json;

struct CmdAst: nix::SourceExprCommand {
	CmdAst() {}

	Category category() override {
		return nix::catUtility;
	}

	std::string description() override {
		return "show a JSON representation of a Nix expression";
	}

	std::string doc() override {
		return
#include "ast.md"
			;
	}

	void run(nix::ref<nix::Store>) override {
		nix::evalSettings.pureEval = false;
		auto eval_state = getEvalState();
		if (this->expr && this->file) {
			throw nix::UsageError("`--file` and `--expr` are mutually exclusive");
		}
		if (!this->file && !this->expr) {
			this->file = "-";
		}
		nix::Expr* expr;
		if (this->file) {
			expr = eval_state->parseExprFromFile(resolve_path(*eval_state, this->file.value()));
		} else {
			expr = eval_state->parseExprFromString(this->expr.value(), resolve_path(*eval_state, "."));
		}
		json j = WithSymbols<nix::Expr*>(expr, eval_state->symbols);
		std::cout << j << std::endl;
	}
};

static auto rCmdAst = nix::registerCommand<CmdAst>("ast");
