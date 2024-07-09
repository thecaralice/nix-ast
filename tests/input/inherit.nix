let
  foo = "hello";
  bar = {
    baz = "world";
  };
in
{
  inherit foo;
  inherit (bar) baz;
}
