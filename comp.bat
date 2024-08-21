clang-format.exe -i src/main.c -style=microsoft
gcc src/main.c -IC:\DEVSOFT\SDL2\include -LC:\DEVSOFT\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -lws2_32 -o main