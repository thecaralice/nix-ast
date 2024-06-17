project(
  'nix-ast',
  'cpp',
  version: '0.1.0',
  default_options: ['warning_level=3', 'cpp_std=c++20'],
)

nix_cmd = dependency('nix-cmd')
nix_expr = dependency('nix-expr')

library(
  'nix-ast',
  'src/plug.cpp',
  dependencies: [nix_cmd, nix_expr],
  install: true,
)
