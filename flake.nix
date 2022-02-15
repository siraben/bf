{
  description = "Brainfuck interpreter";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }:
    utils.lib.eachDefaultSystem (system:
      with import nixpkgs { inherit system; }; {
        defaultPackage = stdenv.mkDerivation {
          name = "bf";
          buildPhase = ''
            cc -o bf bf.c
          '';
          installPhase = ''
            install -Dm755 -t $out/bin bf
          '';
          src = ./.;
        };
      }
    );
}
