{
  description = "MPD queue api";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils, neovim, ... }@inputs:
    let
      lib = import ./lib.nix;
      mkNeovim = lib.mkNeovim;
      mkNeovimPlugins = lib.mkNeovimPlugins;
      plugins = [
        "nvim-which-key"
      ];
    in
    {
      overlay = final: prev: {
        mpdqueueapi = final.callPackage (import ./default.nix) {};
      };
    } // utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; overlays = [ self.overlay ]; };
      in
      rec {
        packages = {
          mpdqueueapi = pkgs.mpdqueueapi;
          lean-language-server = pkgs.lean-language-server;
        };

        defaultPackage = packages.mpdqueueapi;

        defaultApp = {
          type = "app";
          program = "${packages.mpdqueueapi}/bin/mpdqueueapi";
        };

        devShell = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [ libmpdclient mpdqueueapi mpc_cli jq fx ];
        };
      });
}
