CPP         := g++.exe
CC			:= gcc
CFLAGS		:= -Wall -Wextra -Werror -Ofast -std=gnu99 -m32
SRC			:= $(wildcard src/*.c)
OBJ			:= $(SRC:%.c=%.o)
BIN			:= rle
BIN_PATH	:= /usr/local/bin
LIBS        := -L"V:/PAPP/PortableApps/Dev-Cpp/MinGW64/x86_64-w64-mingw32/lib32" -static-libgcc -m32
INCS     	:= -I"V:/PAPP/PortableApps/Dev-Cpp/MinGW64/include" -I"V:/PAPP/PortableApps/Dev-Cpp/MinGW64/x86_64-w64-mingw32/include" -I"V:/PAPP/PortableApps/Dev-Cpp/MinGW64/lib/gcc/x86_64-w64-mingw32/4.9.2/include"
CXXINCS  	:= -I"V:/PAPP/PortableApps/Dev-Cpp/MinGW64/include" -I"V:/PAPP/PortableApps/Dev-Cpp/MinGW64/x86_64-w64-mingw32/include" -I"V:/PAPP/PortableApps/Dev-Cpp/MinGW64/lib/gcc/x86_64-w64-mingw32/4.9.2/include" -I"V:/PAPP/PortableApps/Dev-Cpp/MinGW64/lib/gcc/x86_64-w64-mingw32/4.9.2/include/c++"
CXXFLAGS 	:= $(CXXINCS) -m32


$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -I./inc $(INCS) $(OBJ) -o $@ $(LIBS)

clean:
	rm -f $(OBJ) $(BIN)

install: $(BIN)
	install rle $(BIN_PATH)

re: clean $(BIN)

uninstall:
	rm -f $(BIN_PATH)/$(BIN)

src/%.o: src/%.c inc/rle.h
	$(CC) $(CFLAGS) $(INCS) $(CXXFLAGS) -I./inc -c $< -o $@
