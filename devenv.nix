{
  pkgs,
  ...
}:
{
  languages.c.enable = true;
  packages = [ pkgs.lldb ];
}
