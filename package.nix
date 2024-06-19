{
  stdenv,
  nix,
  boost,
  meson,
  ninja,
  pkg-config,
}:
stdenv.mkDerivation {
  pname = "nix-ast";
  version = "0.1.0";
  src = ./.;
  buildInputs = [
    nix.dev
    boost.dev
  ];
  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];
}