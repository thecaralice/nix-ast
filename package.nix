{
  stdenv,
  nix,
  boost,
  meson,
  ninja,
  pkg-config,
}:
stdenv.mkDerivation {
  name = "nix-ast";
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
