{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-parts.url = "github:hercules-ci/flake-parts";
    nix-github-actions = {
      url = "github:nix-community/nix-github-actions";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      self,
      flake-parts,
      nixpkgs,
      nix-github-actions,
      ...
    }@inputs:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [ ];
      systems = nixpkgs.lib.platforms.all;
      flake.githubActions = nix-github-actions.lib.mkGithubMatrix {
        checks = nixpkgs.lib.getAttrs [
          "x86_64-linux"
          "x86_64-darwin"
        ] self.packages;
      };
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
