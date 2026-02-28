CC_LINUX = gcc
CC_WIN   = x86_64-w64-mingw32-gcc
CFLAGS     = -Wall -Wextra -std=c99 -O2 -Isrc -Ilib -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-sign-compare -Wno-stringop-truncation -Wno-format-truncation -Wno-maybe-uninitialized
CFLAGS_WIN = $(CFLAGS) -DCURL_STATICLIB -static-libgcc -fno-stack-protector

LIB_LIN_PATH = -Ilib/raylib_lin/include -Llib/raylib_lin/lib
LIB_WIN_PATH = -Ilib/raylib_win/include -Llib/raylib_win/lib -I/usr/x86_64-w64-mingw32/include -L/usr/x86_64-w64-mingw32/lib

SRC       = src/main.c src/astro.c src/config.c src/ui.c
OBJ       = $(SRC:src/%.c=build/%.o)

LDFLAGS_LIN = $(LIB_LIN_PATH) -lraylib -lcurl -lGL -lm -lpthread -ldl -lrt -lX11
LDFLAGS_WIN = $(LIB_WIN_PATH) -lraylib -Wl,-Bstatic -lcurl -lnghttp2 -lnghttp3 -lngtcp2 -lngtcp2_crypto_openssl -lssl -lcrypto -lssh2 -lz -lbrotlidec -lbrotlicommon -lidn2 -lpsl -lunistring -liconv -lssp_nonshared -Wl,-Bdynamic -lzstd -lbcrypt -lwldap32 -lws2_32 -lcrypt32 -lsecur32 -liphlpapi -lopengl32 -lgdi32 -lwinmm -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive,--allow-multiple-definition -mwindows
DIST_LINUX = dist/TLEscope-Linux
DIST_WIN   = dist/TLEscope-Windows

.PHONY: all linux windows clean build bin

all: linux

linux: bin/TLEscope
	@mkdir -p $(DIST_LINUX)
	cp bin/TLEscope $(DIST_LINUX)/
	cp -r themes/ $(DIST_LINUX)/
	cp settings.json $(DIST_LINUX)/ 2>/dev/null || true 
	cp data.tle $(DIST_LINUX)/ 2>/dev/null || true
	cp logo*.png $(DIST_LINUX)/ 2>/dev/null || true
	@echo "Linux build bundled in $(DIST_LINUX)/, do not run bin/*"
	@echo "Here's your subshell command to run it! (cd $(DIST_LINUX)/ && ./TLEscope)"

windows: bin/TLEscope.exe
	@mkdir -p $(DIST_WIN)
	cp bin/TLEscope.exe $(DIST_WIN)/
	cp /usr/x86_64-w64-mingw32/bin/libzstd*.dll $(DIST_WIN)/ 2>/dev/null || true
	cp -r themes/ $(DIST_WIN)/
	cp settings.json $(DIST_WIN)/ 2>/dev/null || true
	cp data.tle $(DIST_WIN)/ 2>/dev/null || true
	cp logo*.png $(DIST_WIN)/ 2>/dev/null || true
	cp /usr/x86_64-w64-mingw32/bin/libssp*.dll $(DIST_WIN)/ 2>/dev/null || true
	@echo "Windows build bundled in $(DIST_WIN)/, run it from there!"

# yes makefile this data copied juuuuuuuust fine and is safe and sound don't worry about it :3 
# microsoft, and I mean this sincerely, please keep bloating windows so that people stop using it and annoying me about it thanks bye.

bin/TLEscope: $(OBJ) | bin
	$(CC_LINUX) $(CFLAGS) -o $@ $^ $(LDFLAGS_LIN)

bin/TLEscope.exe: $(SRC) | bin
	$(CC_WIN) $(CFLAGS_WIN) -o $@ $^ $(LDFLAGS_WIN)

build/%.o: src/%.c | build
	$(CC_LINUX) $(CFLAGS) $(LIB_LIN_PATH) -c $< -o $@

build:
	mkdir -p build

bin:
	mkdir -p bin

clean:
	rm -rf build bin dist