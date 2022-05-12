{
  description = "MPD websocket api";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils, ... }@inputs:
    let
      pkg = { asio_1_10, cmake, docopt_cpp, lib, libmpdclient, pkg-config, stdenv, websocketpp }:
        with lib; stdenv.mkDerivation rec {
          pname = "mpdws";
          version = "0.2.0";
          src = ./.;
          enableParallelBuilding = true;
          buildInputs = [ asio_1_10 libmpdclient websocketpp docopt_cpp ];
          nativeBuildInputs = [ pkg-config cmake ];
       
          installPhase = ''
            mkdir -p $out/bin
            chmod +x ./mpdws
            cp ./mpdws $out/bin
          '';
       
          meta = {
            description = "MPD WebSocket API";
            license     = licenses.mit;
            platforms   = platforms.all;
          };
        };
    in
    {
      overlay = final: prev: rec {
        mpdws = final.callPackage pkg {};
      };
      nixosModule = { config, lib, pkgs, ... }: let
        cfg = config.services.mpdws;
      in {
        options.services.mpdws = with lib; {
          enable = mkEnableOption "Enable mpdws, the MPD WebSocket API.";
          host = mkOption  {
              type = types.str;
              default = "127.0.0.1";
              description = "Listen address.";
          };
          port = mkOption {
              type = types.port;
              default = 9001;
              description = "Listen port.";
          };
          mpdHost = mkOption {
            type = types.str;
            default = "127.0.0.1";
            description = "MPD host.";
          };
          mpdPort = mkOption {
            type = types.port;
            default = 6600;
            description = "MPD host.";
          };
        };
        config = lib.mkIf cfg.enable {
          systemd.services.mpdws = {
            after = [ "network.target" ];
            description = "MPD WebSocket API";
            wantedBy = "multi-user.target";
            serviceConfig = {
              ExecStart = "${pkgs.mpdws}/bin/mpdws --host=${cfg.host} --port=${toString cfg.port} --mpd-host=${cfg.mpdHost} --mpd-port=${cfg.mpdPort}";
              Restart = "always";
            };
          };
        }; 
      };
    } // utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; overlays = [ self.overlay ]; };
      in
      rec {
        packages = {
          mpdws = pkgs.mpdws;
        };

        defaultPackage = packages.mpdws;

        apps = {
          mpdws = {
            type = "app";
            program = "${packages.mpdws}/bin/mpdws";
          };
        };

        defaultApp = apps.mpdws;

        devShell = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            docopt_cpp
            pkg-config
            asio_1_10
            websocketpp
            libmpdclient
            jq
            fx
          ];
        };
      });
}
