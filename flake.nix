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
          inherit (lib) flip const;
          compose = flip lib.pipe;
          mapAttrValues = compose [
            const
            lib.mapAttrs
          ];
          nv = {
            inherit (pkgs.nixVersions) nix_2_18 nix_2_19 nix_2_20;
            default = pkgs.nix;
          };
        in
        mapAttrValues (lib.flip mapAttrValues nv) {
          devShells =
            nix:
            pkgs.mkShell {
              packages =
                (with pkgs; [
                  boost.dev
                  meson
                  ninja
                  pkg-config
                  clang-tools
                ])
                ++ [ nix.dev ];
            };
          packages = nix: pkgs.callPackage ./package.nix { inherit nix; };
        };
    };
}
