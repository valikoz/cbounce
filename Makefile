# CFLAGS= -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -Wall -Wextra -g
# RAYLIB_IN= -I./raylib/raylib-5.5_macos/include
# RAYLIB_LD= -L./raylib/raylib-5.5_macos/lib -Wl,-rpath,./raylib/raylib-5.5_macos/lib -lraylib # libraylib.a
CFLAGS= -Wall -Wextra -I/usr/local/include -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -g
RAYLIB_IN= -I./raylib/raylib-6.2_linux_amd64/include
RAYLIB_LD= -L./raylib/raylib-6.2_linux_amd64/lib -Wl,-rpath=./raylib/raylib-6.2_linux_amd64/lib -lraylib

# main: main.c game.dylib
# 	cc $(CFLAGS) $(RAYLIB_IN) $(RAYLIB_LD) -Wl,-rpath,./ -o main main.c

main: main.c game.so
	cc -o main main.c $(CFLAGS) $(RAYLIB_IN) $(RAYLIB_LD) -Wl,-rpath=./

# game.dylib: game.c game.h
# 	cc $(CFLAGS) $(RAYLIB_IN) $(RAYLIB_LD) -fPIC -shared -o ./game.dylib game.c
game.so: game.c game.h
	cc -fPIC -shared -o ./game.so game.c $(CFLAGS) $(RAYLIB_IN) $(RAYLIB_LD)
