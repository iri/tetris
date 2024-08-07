clang-format.exe -i src/main.c -style=Microsoft
gcc -std=c11 src/main.c -IC:\DEVSOFT\SDL2\include -LC:\DEVSOFT\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -o main