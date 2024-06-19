# nix-ast

`nix-ast` shows the JSON representation of a Nix expression on standard output, made for checking potential backwards incompatibility in nixos/nix#9971.

## Requirements

Nix C++ interface is unstable, this is only compatible with Nix versions from 2.18 to 2.20 inclusive.

## Installation

This repo is a flake providing a single package output.

### Command-line (non-persistent)

```sh
$ nix build github:thecaralice/nix-ast#default
$ nix --plugin-files ./result/lib/libnix-ast.dylib ast --expr 'let name = "world"; in "Hello, ${name}"'
{"attrs":{"attrs":{"name":{"inherited":false,"value":{"kind":"String","value":"world"}}},"dynamic":[],"kind":"Attrs","recursive":false},"body":{"forceString":true,"kind":"ConcatStrings","strings":[{"kind":"String","value":"Hello, "},{"displacement":0,"from_with":null,"kind":"Var","level":0,"name":"name"}]},"kind":"Let"}
```

### NixOS/nix-darwin/Home Manager configuration

```nix
# In flake.nix
{
  inputs.nix-ast.url = "github:thecaralice/nix-ast";
}
```
```nix
# in configuration
{
  nix.settings.plugin-files = [ "${inputs.nix-ast.packages.${system}.default}/lib/libnix.dylib" ]; # or .so for linux
}
```
