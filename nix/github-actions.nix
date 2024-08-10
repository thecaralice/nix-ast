{
  inputs,
  lib,
  config,
  flake-parts-lib,
  ...
}:
let
  inherit (lib) types;
  inherit (inputs.nix-github-actions.lib) mkGithubMatrix githubPlatforms;
  transpose = lib.foldlAttrs (
    acc: name: value:
    lib.pipe value [
      (lib.mapAttrs (_: x: { ${name} = x; }))
      (lib.recursiveUpdate acc)
    ]
  ) { };
in
{
  options = {
    github-actions.attrPrefix = lib.mkOption {
      type = types.str;
      default = "githubActions.checks";
    };
    perSystem = flake-parts-lib.mkPerSystemOption (
      { self', system, ... }:
      {
        options.github-actions = {
          checks = lib.mkOption {
            type = types.lazyAttrsOf types.package;
            default = self'.checks;
            example = self'.packages;
          };
          platform = lib.mkOption {
            type = types.nullOr types.str;
            default = githubPlatforms.${system} or null;
          };
        };
      }
    );
  };
  config.flake.githubActions =
    let
      trans = lib.pipe config.allSystems [
        (lib.mapAttrs (_: x: x.github-actions))
        (lib.filterAttrs (_: x: x.platform != null))
        transpose
      ];
    in
    mkGithubMatrix {
      inherit (config.github-actions) attrPrefix;
      inherit (trans) checks;
      platforms = trans.platform;
    };
}
