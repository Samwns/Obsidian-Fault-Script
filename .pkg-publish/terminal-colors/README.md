# @samwns/ofs-lib-terminal-colors

OFS library package published via GitHub Packages.

Main module: libs/terminal_colors.ofs

Usage in OFS:

attach "terminal_colors.ofs"

Examples:

echo_red("foreground only")
echo_on_blue("white text on blue background")
color_bg_echo(ansi_yellow(), ansi_bg_purple(), "yellow on purple")
