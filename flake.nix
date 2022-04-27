{
  description = "MPD queue api";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils, ... }@inputs:
    let
      version = "0.1.0";
      pkg = { lib , stdenv , libmpdclient , readline }: with lib; stdenv.mkDerivation rec {
        inherit version;
        pname = "mpdqueueapi";
        src = ./.;
        enableParallelBuilding = true;
        buildInputs = [ libmpdclient ];
      
        installPhase = ''                                                                               
          mkdir -p $out/bin                                                                             
          chmod +x ./mpdqueueapi
          cp ./mpdqueueapi $out/bin
        '';
      
        meta = {
          description = "MPD queue api";
          license     = licenses.mit;
          platforms   = platforms.all;
        };
      };
      server_pkg = { lib, stdenv, libmpdclient, mpdqueueapi, python38Packages, ... }: with lib; stdenv.mkDerivation {
        inherit version;
        pname = "mpdqueueapi-server";
        src = ./.;
        buildInputs = with python38Packages; [ libmpdclient mpdqueueapi flask ];
        installPhase = ''
          mkdir -p $out/bin
          ls
          chmod +x ./src/server.py
          cp ./src/server.py $out/bin/mpdqueueapi-server
        '';
        meta = {
          description = "MPD queue api";
          license     = licenses.mit;
          platforms   = platforms.all;
        };
      };
    in
    {
      overlay = final: prev: rec {
        mpdqueueapi = final.callPackage pkg {};
        mpdqueueapi-server = final.callPackage server_pkg { inherit mpdqueueapi; };
      };
    } // utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; overlays = [ self.overlay ]; };
      in
      rec {
        packages = {
          mpdqueueapi = pkgs.mpdqueueapi;
          mpdqueueapi-server = pkgs.mpdqueueapi-server;
        };

        defaultPackage = packages.mpdqueueapi;

        apps = {
          mpdqueueapi = {
            type = "app";
            program = "${packages.mpdqueueapi}/bin/mpdqueueapi";
          };
          mpdqueueapi-server = {
            type = "app";
            program = "${packages.mpdqueueapi-server}/bin/mpdqueueapi-server";
          };
        };

        defaultApp = apps.mpdqueueapi;

        devShell = pkgs.mkShell {
          nativeBuildInputs = with pkgs; with pkgs.python38Packages; [
            libmpdclient
            mpdqueueapi
            mpdqueueapi-server
            flask
            mpc_cli
            jq
            fx
          ];
        };
      });
}
