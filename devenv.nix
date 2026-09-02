{
  lib,
  pkgs,
  ...
}:
{
  languages.c.enable = true;
  packages = [ pkgs.lldb ];
  env.LLDB_DEBUGSERVER_PATH =
    lib.mkIf pkgs.stdenv.hostPlatform.isDarwin "/Library/Developer/CommandLineTools/Library/PrivateFrameworks/LLDB.framework/Resources/debugserver";
}
