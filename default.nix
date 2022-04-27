{ lib
, stdenv
, libmpdclient
, readline
}:

with lib;
stdenv.mkDerivation rec {
  pname = "mpdqueueapi";
  version = "0.1.0";

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
    # homepage    = "";
    license     = licenses.mit;
    platforms   = platforms.all;
  };
}
