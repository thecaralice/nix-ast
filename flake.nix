{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs =
    { flake-parts, nixpkgs, ... }@inputs:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [ ];
      systems = nixpkgs.lib.platforms.all;
      perSystem =
        {
          pkgs,
          system,
          self',
          lib,
          ...
        }:
        let
          nix = pkgs.nixVersions.nix_2_20;
        in
        {
          packages.default = pkgs.stdenv.mkDerivation {
            name = "nix-ast";
            src = ./.;
            buildInputs = [
              nix.dev
              pkgs.boost.dev
            ];
            nativeBuildInputs = [
              pkgs.meson
              pkgs.ninja
              pkgs.pkg-config
            ];
          };

          devShells.default = pkgs.mkShell {
            packages = [
              nix.dev
              pkgs.boost.dev
              pkgs.meson
              pkgs.ninja
              pkgs.pkg-config
              pkgs.clang-tools
            ];
          };
        };
    };
}
