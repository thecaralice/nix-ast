nix_cmd = dependency('nix-cmd')
nix_expr = dependency('nix-expr')

nix_version = nix_expr.version().split('.')

plugin = library(
	'nix-ast',
	'plug.cpp',
	'expr_json.cpp',
	cpp_args: [
		'-DNIX_VERSION_MAJOR=' + nix_version[0],
		'-DNIX_VERSION_MINOR=' + nix_version[1],
		'-DNIX_VERSION_PATCH=' + nix_version[2],
	],
	dependencies: [nix_cmd, nix_expr],
	install: true,
)
