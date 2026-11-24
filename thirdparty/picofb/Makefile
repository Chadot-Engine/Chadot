linux_x11:
	gcc benchmark.c -o benchmark_x11 -lX11 -lXext -Wall -Wextra -O3

linux_wayland: xdg-shell-client-protocol.h xdg-shell-client-protocol.c
	gcc benchmark.c -o benchmark_wayland xdg-shell-client-protocol.c -DPICOFB_WAYLAND -lwayland-client -Wall -Wextra -O3

windows:
	gcc benchmark.c -o benchmark_win32.exe -lgdi32 -luser32 -Wall -Wextra -O3

windows_cross:
	x86_64-w64-mingw32-gcc benchmark.c -o benchmark_win32.exe -lgdi32 -luser32 -Wall -Wextra -O3

other:
	gcc benchmark.c -o benchmark_sdl -DPICOFB_BACKEND_OVERRIDE -DPICOFB_SDL_BACKEND -lSDL3 -Wall -Wextra -O3


xdg-shell-client-protocol.h:
	./generate_wayland_protocols.sh
xdg-shell-client-protocol.c:
	./generate_wayland_protocols.sh
