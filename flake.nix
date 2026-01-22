{
  description = "openMPI and openMP development environment for NixOS";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config.allowUnfree = true;
        };

      in {
        devShells.default = pkgs.mkShell {
            packages = with pkgs; [
                gcc
                gnumake
                valgrind
                gsl
                mpi 
                rocmPackages.mpi 
                rocmPackages.llvm.openmp
            ];
            #shellHook = 
            #;
        };
      });
}
