{
  description = "MPD queue api";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils, ... }@inputs:
    let
      version = "0.1.0";
      pkg = { lib, stdenv, cmake, asio_1_10, libmpdclient, websocketpp, pkg-config }: with lib; stdenv.mkDerivation rec {
        inherit version;
        pname = "mpdqueueapi";
        src = ./.;
        enableParallelBuilding = true;
        buildInputs = [ asio_1_10 libmpdclient websocketpp ];
        nativeBuildInputs = [ pkg-config cmake ];
      
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
    in
    {
      overlay = final: prev: rec {
        mpdqueueapi = final.callPackage pkg {};
      };
    } // utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; overlays = [ self.overlay ]; };
      in
      rec {
        packages = {
          mpdqueueapi = pkgs.mpdqueueapi;
        };

        defaultPackage = packages.mpdqueueapi;

        apps = {
          mpdqueueapi = {
            type = "app";
            program = "${packages.mpdqueueapi}/bin/mpdqueueapi";
          };
        };

        defaultApp = apps.mpdqueueapi;

        devShell = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            pkg-config
            asio_1_10
            websocketpp
            libmpdclient
            mpdqueueapi
            jq
            fx
          ];
        };
      });
}
