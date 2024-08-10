{
  perSystem =
    { pkgs, ... }:
    {
      checks.editorconfig =
        pkgs.runCommand "ec-check"
          {
            src = ./.;
            nativeBuildInputs = [ pkgs.editorconfig-checker ];
          }
          ''
            cd "$src"
            editorconfig-checker -verbose -format gcc | tee "$out"
          '';
    };
}
