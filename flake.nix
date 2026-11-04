{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

    zig-flake = {
      url = "github:silversquirl/zig-flake";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = inputs@{ self, nixpkgs, zig-flake }:
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
                ;

              system = stdenv.hostPlatform.system;

              zig = zig-flake.packages.${system}.zig_0_17_1;
            in {
              devShells.${system}.default = mkShell {
                packages = with pkgs; [
                  zig
                ];
              };
            })
        ));
}
