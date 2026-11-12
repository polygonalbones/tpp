{
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs = inputs@{ self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
      ];
      forAllSystems = function: nixpkgs.lib.genAttrs systems (system:
        function nixpkgs.legacyPackages.${system});
    in
      nixpkgs.lib.foldAttrs nixpkgs.lib.mergeAttrs {} (
        nixpkgs.lib.mapAttrsToList (_: v: v) (
          forAllSystems (pkgs:
            let
              inherit (pkgs)
                stdenv
                mkShell
                fetchFromGitHub
                ;

              system = stdenv.hostPlatform.system;
            in {
              devShells.${system}.default = mkShell {
                packages = with pkgs; [
                  # C compilers to test which ones can compile tpp
                  gcc
                  clang
                  tinycc
                ];
              };
            })
        ));
}
