linux_x11:
	gcc doom_fire.c -o doom_fire -lX11 -lXext -Wall -Wextra -O3
	gcc random_clouds.c -o random_clouds -lX11 -lXext -Wall -Wextra -O3

linux_wayland: xdg-shell-client-protocol.h xdg-shell-client-protocol.c
	gcc doom_fire.c -o doom_fire xdg-shell-client-protocol.c -DPICOFB_WAYLAND -lwayland-client -Wall -Wextra -O3
	gcc random_clouds.c -o random_clouds xdg-shell-client-protocol.c -DPICOFB_WAYLAND -lwayland-client -Wall -Wextra -O3

windows:
	gcc doom_fire.c -o doom_fire.exe -lgdi32 -luser32 -Wall -Wextra -O3
	gcc random_clouds.c -o random_clouds.exe -lgdi32 -luser32 -Wall -Wextra -O3

windows_cross:
	x86_64-w64-mingw32-gcc doom_fire.c -o doom_fire.exe -lgdi32 -luser32 -Wall -Wextra -O3
	x86_64-w64-mingw32-gcc random_clouds.c -o random_clouds.exe -lgdi32 -luser32 -Wall -Wextra -O3

other:
	gcc doom_fire.c -o doom_fire -DPICOFB_BACKEND_OVERRIDE -DPICOFB_SDL_BACKEND -lSDL3 -Wall -Wextra -O3
	gcc random_clouds.c -o random_clouds -DPICOFB_BACKEND_OVERRIDE -DPICOFB_SDL_BACKEND -lSDL3 -Wall -Wextra -O3


xdg-shell-client-protocol.h:
	./generate_wayland_protocols.sh
xdg-shell-client-protocol.c:
	./generate_wayland_protocols.sh
