{
  stdenv,
  nix,
  boost,
  meson,
  ninja,
  pkg-config,
  jq,
  lib,
}:
stdenv.mkDerivation {
  pname = "nix-ast";
  version = "0.1.0";
  src = lib.fileset.toSource {
    root = ./.;
    fileset = lib.fileset.unions [
      ./meson.build
      ./src
      ./tests
    ];
  };
  buildInputs = [
    nix.dev
    boost.dev
  ];
  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];
  nativeCheckInputs = [ jq ];
  doCheck = true;
}
