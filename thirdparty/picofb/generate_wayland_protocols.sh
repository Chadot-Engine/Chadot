#!/bin/sh

set -e

XDG_SHELL=$(pkg-config --variable=pkgdatadir wayland-protocols)/stable/xdg-shell/xdg-shell.xml

if [ ! -f "$XDG_SHELL" ]; then
    echo "Error: xdg-shell.xml not found"
    echo "Install wayland-protocols: sudo apt install wayland-protocols"
    exit 1
fi

wayland-scanner client-header "$XDG_SHELL" xdg-shell-client-protocol.h
wayland-scanner private-code "$XDG_SHELL" xdg-shell-client-protocol.c

echo "Generated:"
echo "  xdg-shell-client-protocol.h"
echo "  xdg-shell-client-protocol.c"