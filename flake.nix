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
    { flake-parts, nixpkgs, ... }@inputs:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [
        ./nix/ec-check.nix
        ./nix/github-actions.nix
      ];
      systems = nixpkgs.lib.platforms.all;
      perSystem =
        {
          pkgs,
          self',
          lib,
          ...
        }:
        let
          inherit (lib) flip const;
          compose = flip lib.pipe;
          mapAttrValues = compose [
            const
            lib.mapAttrs
          ];
          nv = {
            inherit (pkgs.nixVersions)
              nix_2_18
              nix_2_19
              nix_2_20
              nix_2_21
              nix_2_22
              ;
            default = pkgs.nix;
          };
          llvm = pkgs.llvmPackages_18;
        in
        mapAttrValues (lib.flip mapAttrValues nv) {
          devShells =
            nix:
            pkgs.mkShell.override { inherit (llvm) stdenv; } {
              packages =
                (with pkgs; [
                  boost.dev
                  meson
                  ninja
                  pkg-config
                ])
                ++ (with llvm; [ clang-tools ])
                ++ [ nix.dev ];
            };
          packages =
            nix:
            pkgs.callPackage ./package.nix {
              inherit nix;
              inherit (llvm) stdenv;
            };
        }
        // {
          github-actions.checks = self'.packages // self'.checks;
        };
    };
}
