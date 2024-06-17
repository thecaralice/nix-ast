R""(

# Examples

* Get a JSON representation of a Nix expression:

  ```console
  # nix ast --expr 'let name = "world"; in "Hello, ${name}"'
  {"attrs":{"attrs":{"name":{"inherited":false,"value":{"kind":"String","value":"world"}}},"dynamic":[],"kind":"Attrs","recursive":false},"body":{"forceString":true,"kind":"ConcatStrings","strings":[{"kind":"String","value":"Hello, "},{"displacement":0,"from_with":null,"kind":"Var","level":0,"name":"name"}]},"kind":"Let"}
  ```

* Get a JSON representation of a Nix file:
  ```console
  # nix ast --file ./file.nix
  ```

# Description

This commands shows the JSON representation of a Nix expression on standard output.

)""
