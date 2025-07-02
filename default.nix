{ pkgs, stdenv, lib }:

stdenv.mkDerivation rec {
  name = "ds4ctl-${version}";
  version = "0.1";

  src = ./.;

  makeFlags = "PREFIX=$(out)";

  nativeBuildInputs = with pkgs; [ clang ];
  buildInputs = [ ];

  meta = with lib; {
    description = "ds4ctl";
    homepage = "https://github.com/jb55/ds4ctl";
    maintainers = with maintainers; [ jb55 ];
    license = licenses.mit;
  };
}
