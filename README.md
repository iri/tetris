# TETRIS

## Requirements
Windows  
Mingw32  
SDL2  

## Compile
`gcc src/main.c -I<SDL2-dir>\include -L<SDL2-dir>\lib -Wall -lmingw32 -lSDL2main -lSDL2 -o tetris`  
(or run comp.bat)

## Usage
`tetris.exe -d 1200 800 -r`  
(or run start.bat)

Controls:
 - Start - "Space";
 - Move - "Left","Right","Up","Space"
 - Quit - "Q"
