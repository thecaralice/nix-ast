#include <nix/eval.hh>

inline nix::SourcePath resolve_path(nix::EvalState& eval_state, std::string_view path) {
#if NIX_VERSION_MINOR < 21
	return eval_state.rootPath(nix::CanonPath::fromCwd(path));
#else
	return eval_state.rootPath(path);
#endif
}
