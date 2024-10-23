#include <SDL.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>
#include <windows.h>

/**
 * @brief Timer data structure type
 *
 */
typedef struct _ttimer
{
    DWORD last;     //  Timestamp when the timer was fixed last time (in ms since program start)
    int ms;         //  Timer value in ms when tick must be triggered
} tTimer;

/**
 * @brief Initialize / reinitialize timer
 *
 * @param t : Timer
 */
void timer_start(tTimer *t);

/**
 * @brief Return current value of the timer t (in ms)
 *
 * @param t : Timer
 * @return uint64_t Timer value
 */
DWORD timer_check(tTimer *t);

/**
 * @brief Is time to tick the timer?
 *
 * @param t : Timer
 * @return true : Yes
 * @return false : No
 */
bool is_timer_tick(tTimer *t);

/**
 * @brief Program configuration data structure
 *
 */
typedef struct _tconfig
{
    int x, y; // Coordinates of window left-top corner
    int w, h; // Window width, window height
} tConfig;

/**
 * @brief Constants for game state
 *
 */
#define MAXITEMS 7
#define MAXCOLORS 7
#define ITEMBLOCKS 4
#define GLASS_W 14
#define GLASS_H 28

/**
 * @brief Possible game state variable values
 *
 */
typedef enum
{
    /*
    @startuml
        [*] --> GAME_WELCOME
        GAME_WELCOME --> GAME_STARTED
        GAME_STARTED --> ITEM_STARTED
        ITEM_STARTED --> ITEM_FALLING
        ITEM_FALLING --> ITEM_FALLING_FAST
        ITEM_FALLING --> ITEM_FALLING
        ITEM_FALLING --> ITEM_STOPPED
        ITEM_FALLING_FAST --> ITEM_STOPPED
        ITEM_FALLING_FAST --> ITEM_FALLING_FAST
        ITEM_STOPPED --> ITEM_STARTED
        ITEM_STOPPED --> GAME_FINISHED
        GAME_FINISHED --> GAME_WELCOME
    @enduml
     */
    GAME_WELCOME,      // Game welcome screen
    GAME_STARTED,      // User started game
    ITEM_STARTED,      // An item started to fall
    ITEM_FALLING,      // The item falls slowly
    ITEM_FALLING_FAST, // The item falls fast (user pressed <space>)
    ITEM_STOPPED,      // An Item stopped falling
    GAME_FINISHED      // The glass is full, game over
} tGameState;

/**
 * @brief Return the Game State name
 *
 * @param state : Game state variable value
 * @return const char* : Game state name
 */
const char *getGameState(tGameState state);

/**
 * @brief Game state data structure
 *
 */
typedef struct _tstate
{
    uint8_t colors[MAXCOLORS][3];                    // Array of pPossible item colors
    int8_t items[MAXITEMS][ITEMBLOCKS * ITEMBLOCKS]; // Array of possible item types
    int fps;                                         // Frames per second for screen refresh
    int block_size;                                  // Block size in px
    tGameState GAME_STATE;                           // Game state variable
    tTimer TIMER_FPS;                                // Timer for frame drawing (100/60 ms by default)
    tTimer TIMER_1;                                  // Timer for slow falling (1000 ms by default)
    tTimer TIMER_2;                                  // Timer for fast falling (100 ms by default)
    int ITEM_ID;                                     // Current item
    int glass_x, glass_y;                            // Position of left top corner of glass (in px)
    int glass_w, glass_h;                            // Wisth and heightr of glass (in px)
    uint8_t glass[GLASS_H][GLASS_W];                 // Array for blocks in the glass
    int x, y;                                        // Coordinates of the falling item's left-up corner
    int gx, gy;                     // Glass position corresponding to the left-up block of the item (in blocks)
    int8_t marg_left[ITEMBLOCKS];   // Blocks when the item starts from the left (for each item line)
    int8_t marg_right[ITEMBLOCKS];  // Blocks when the item ends from the left (for each item line)
    int8_t marg_bottom[ITEMBLOCKS]; // Blocks when the item ends from the top (for each item column)
} tState;

/**
 * @brief Find the minimum of the 4 values
 *
 * @return int8_t : Minimum value
 */
int8_t min4(int8_t a, int8_t b, int8_t c, int8_t d);

/**
 * @brief Find the mfximum of the 4 values
 *
 * @return int8_t : Maximum value
 */
int8_t max4(int8_t a, int8_t b, int8_t c, int8_t d);

/**
 * @brief Recompute depending values of item's coordinates
 *
 * @param ST : State data structure
 */
void updState(tState *ST);

/**
 * @brief Get the Bottom Margins of the item
 *
 * @param ST : State data structure
 */
void getBottomMargins(tState *ST);

/**
 * @brief Get the Left Margins of the item
 *
 * @param ST : State data structure
 */
void getLeftMargins(tState *ST);

/**
 * @brief Get the Right Margins of the item
 *
 * @param ST : State data structure
 */
void getRightMargins(tState *ST);

/**
 * @brief Check the touch of the left margins of the item
 *
 * @param ST : State data structure
 * @return true
 * @return false
 */
bool checkItemLeft(tState *ST);

/**
 * @brief Check the touch of the right margins of the item
 *
 * @param ST : State data structure
 * @return true
 * @return false
 */
bool checkItemRight(tState *ST);

/**
 * @brief Check the touch of the bottim margins of the item
 *
 * @param ST : State data structure
 * @return true
 * @return false
 */
bool checkItemBottom(tState *ST);

/**
 * @brief Rotate item left
 *
 * @param ST : State data structure
 */
void rotateItem(tState *ST);

/**
 * @brief Print item array values
 *
 * @param ST : State data structure
 */
void printItem(tState *ST);

/**
 * @brief Copy item's blocks to glass array
 *
 * @param ST : State data structure
 */
void copyBlocksToGlass(tState *ST);

/**
 * @brief Print glass array values
 *
 * @param ST : State data structure
 */
void printGlass(tState *ST);

/**
 * @brief Remove specified line, all lined above move down by one line
 *
 * @param ST : State data structure
 * @param line
 */
void removeFullLine(tState *ST, int line);

/**
 * @brief Remove full lines from the glass
 *
 * @param ST : State data structure
 * @return true
 * @return false
 */
bool checkRemoveFullLine(tState *ST);

/**
 * @brief Get uint argument value
 *
 * @param argc : Number of arguments passed to the program
 * @param argv : Values of arguments passed to the program
 * @param ind : Current argument
 * @param res : Obtained argument value
 * @return true : Successful obtaining of argument
 * @return false : Unsuccessful obtaining of argument
 */
bool parse_opt_arg_uint(int argc, char **argv, int *ind, int *res);

/**
 * @brief Get string argument value
 *
 * @param argc : Number of arguments passed to the program
 * @param argv : Values of arguments passed to the program
 * @param ind : Current argument
 * @param res : Pointer to place for obtained argument (must be allocated)
 * @param len : Maximum length of obtained argument
 * @return true : Successful obtaining of argument
 * @return false : Unsuccessful obtaining of argument
 */
bool parse_opt_arg_str(int argc, char **argv, int *ind, char *res, int len);

/**
 * @brief
 *
 * @param argc : Number of arguments passed to the program
 * @param argv : Values of arguments passed to the program
 * @param conf : Configuration data structure
 * @param DM : Display mode data structure
 */
void parse_args(int argc, char **argv, tConfig *conf, SDL_DisplayMode *DM);

/**
 * @brief Draw item
 *
 * @param rend : Renderer data structure
 * @param x : X coordinate of item
 * @param y : Y coordinate of item
 * @param ST : State data structure
 */
void drawItem(SDL_Renderer *rend, int x, int y, tState *ST);

/**
 * @brief Draw glass with blocks in it
 *
 * @param rend : Renderer data structure
 * @param ST : State data structure
 */
void drawGlass(SDL_Renderer *rend, tState *ST);

/**
 * @brief Initialize glass array
 *
 * @param ST : State data structure
 */
void clearGlass(tState *ST);

/**
 * @brief Process falling step of item
 *
 * @param ST : State data structure
 */
void fallStep(tState *ST);

