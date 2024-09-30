{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-parts.url = "github:hercules-ci/flake-parts";
    flake-gha.url = "github:thecaralice/flake-gha";
    flake-gha.inputs = {
      flake-parts.follows = "flake-parts";
    };
  };

  outputs =
    {
      flake-parts,
      nixpkgs,
      flake-gha,
      ...
    }@inputs:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [
        ./nix/ec-check.nix
        flake-gha.flakeModules.default
      ];
      systems = nixpkgs.lib.platforms.all;
      perSystem =
        {
          pkgs,
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
        mapAttrValues (lib.flip mapAttrValues nv) rec {
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
          apps = nix: {
            type = "app";
            program = lib.getExe (
              pkgs.writeShellApplication {
                name = "nix-with-ast";
                runtimeInputs = [ nix ];
                text = ''
                  exec nix --plugin-files ${lib.escapeShellArg (packages nix)}/lib/ "$@"
                '';
              }
            );
          };
        };
      githubActions = {
        cachix = {
          enable = true;
          cacheName = "nix-ast";
        };
        checkAllSystems = false;
      };
    };
}
