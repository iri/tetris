#include <SDL2/SDL.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <windows.h>

/**
 * @brief Timer data structure type
 *
 */
typedef struct _ttimer
{
    uint64_t last; //  Timestamp when the timer was fixed last time (in ms since program start)
    int ms;        //  Timer value in ms when tick must be triggered
} tTimer;

/**
 * @brief Initialize / reinitialize timer
 *
 * @param t : Timer
 */
void timer_start(tTimer *t)
{
    t->last = GetTickCount();
}
/**
 * @brief Return current value of the timer t (in ms)
 *
 * @param t : Timer
 * @return uint64_t Timer value
 */
uint64_t timer_check(tTimer *t)
{
    return GetTickCount() - t->last;
}
/**
 * @brief Is time to tick the timer?
 *
 * @param t : Timer
 * @return true : Yes
 * @return false : No
 */
bool is_timer_tick(tTimer *t)
{
    return (GetTickCount() - t->last) >= t->ms;
}

/**
 * @brief Program configuration data structure
 *
 */
typedef struct _tconfig
{
    int _l, _r, _t, _b, _f;
    int w, h; // Window width, window height
    int x, y; // Coordinates of window left-top corner
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
const char *getGameState(tGameState state)
{
    switch (state)
    {
    case GAME_WELCOME:
        return "GAME_WELCOME";
    case GAME_STARTED:
        return "GAME_STARTED";
    case ITEM_STARTED:
        return "ITEM_STARTED";
    case ITEM_FALLING:
        return "ITEM_FALLING";
    case ITEM_FALLING_FAST:
        return "ITEM_FALLING_FAST";
    case ITEM_STOPPED:
        return "ITEM_STOPPED";
    case GAME_FINISHED:
        return "GAME_FINISHED";
    }
    return "invalid status";
}

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
inline int8_t min_of_four(int8_t a, int8_t b, int8_t c, int8_t d)
{
    int8_t min = a;

    if (b < min)
    {
        min = b;
    }
    if (c < min)
    {
        min = c;
    }
    if (d < min)
    {
        min = d;
    }

    return min;
}

/**
 * @brief Find the mfximum of the 4 values
 *
 * @return int8_t : Maximum value
 */
inline int8_t max_of_four(int8_t a, int8_t b, int8_t c, int8_t d)
{
    int8_t max = a;

    if (b > max)
    {
        max = b;
    }
    if (c > max)
    {
        max = c;
    }
    if (d > max)
    {
        max = d;
    }

    return max;
}

/**
 * @brief Recompute depending values of item's coordinates
 *
 * @param ST : State data structure
 */
void updState(tState *ST)
{
    ST->gx = (ST->x - ST->glass_x) / ST->block_size;
    ST->gy = (ST->y - ST->glass_y) / ST->block_size;
}

/**
 * @brief Get the Bottom Margins of the item
 *
 * @param ST : State data structure
 */
void getBottomMargins(tState *ST)
{
    for (int i = 0; i < ITEMBLOCKS; i++)
    {
        for (ST->marg_bottom[i] = ITEMBLOCKS - 1; ST->marg_bottom[i] >= 0; ST->marg_bottom[i]--)
        {
            if (ST->items[ST->ITEM_ID][ST->marg_bottom[i] * ITEMBLOCKS + i] > 0)
            {
                break;
            }
        }
    }
}

/**
 * @brief Get the Left Margins of the item
 *
 * @param ST : State data structure
 */
void getLeftMargins(tState *ST)
{
    for (int i = 0; i < ITEMBLOCKS; i++)
    {
        for (ST->marg_left[i] = 0; ST->marg_left[i] < ITEMBLOCKS; ST->marg_left[i]++)
        {
            if (ST->items[ST->ITEM_ID][i * ITEMBLOCKS + ST->marg_left[i]] > 0)
            {
                break;
            }
        }
    }
}

/**
 * @brief Get the Right Margins of the item
 *
 * @param ST : State data structure
 */
void getRightMargins(tState *ST)
{
    for (int i = 0; i < ITEMBLOCKS; i++)
    {
        for (ST->marg_right[i] = ITEMBLOCKS - 1; ST->marg_right[i] >= 0; ST->marg_right[i]--)
        {
            if (ST->items[ST->ITEM_ID][i * ITEMBLOCKS + ST->marg_right[i]] > 0)
            {
                break;
            }
        }
    }
}

/**
 * @brief Check the touch of the left margins of the item
 *
 * @param ST : State data structure
 * @return true
 * @return false
 */
bool checkItemLeft(tState *ST)
{
    for (int i = 0; i < ITEMBLOCKS; i++)
    {
        if (ST->marg_left[i] >= 0)
        {
            if (ST->gx + ST->marg_left[i] <= 0 || ST->glass[ST->gy + i + 0][ST->gx + ST->marg_left[i] - 1] > 0)
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Check the touch of the right margins of the item
 *
 * @param ST : State data structure
 * @return true
 * @return false
 */
bool checkItemRight(tState *ST)
{

    for (int i = 0; i < ITEMBLOCKS; i++)
    {
        if (ST->marg_right[i] >= 0)
        {
            if (ST->gx + ST->marg_right[i] >= GLASS_W - 1 ||
                ST->glass[ST->gy + i + 0][ST->gx + ST->marg_right[i] + 1] > 0)
            {
                return true;
            }
        }
    }
    return false;
}

// int checkItemLeftInt(tState *ST, int gx_min)
// {
//     int r = 100;
//     int r1;
//     for (int i = 0; i < ITEMBLOCKS; i++)
//     {
//         if (ST->marg_left[i] >= 0)
//         {
//             r1 = ST->gx + ST->marg_left[i] - gx_min;
//             if (r1 < r)
//             {
//                 r = r1;
//             }
//         }
//     }
//     return r;
// }

// int checkItemRightInt(tState *ST, int gx_max)
// {
//     int r = -100;
//     int r1;
//     for (int i = 0; i < ITEMBLOCKS; i++)
//     {
//         if (ST->marg_right[i] >= 0)
//         {
//             r1 = gx_max - (ST->gx + ST->marg_right[i]);
//             if (r1 > r)
//             {
//                 r = r1;
//             }
//         }
//     }
//     return r;
// }

/**
 * @brief Check the touch of the bottim margins of the item
 *
 * @param ST : State data structure
 * @return true
 * @return false
 */
bool checkItemBottom(tState *ST)
{
    for (int i = 0; i < ITEMBLOCKS; i++)
    {
        if (ST->marg_bottom[i] >= 0)
        {
            if (ST->gy + ST->marg_bottom[i] + 2 > GLASS_H || ST->glass[ST->gy + ST->marg_bottom[i] + 1][ST->gx + i] > 0)
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Rotate item left
 *
 * @param ST : State data structure
 */
void rotateItem(tState *ST)
{
    int8_t *mat = ST->items[ST->ITEM_ID];
    for (int8_t i = 0; i < ITEMBLOCKS / 2; i++)
    {
        for (int8_t j = i; j < ITEMBLOCKS - i - 1; j++)
        {
            // Calculate the indices in the one-dimensional array
            int8_t index1 = i * ITEMBLOCKS + j;
            int8_t index2 = j * ITEMBLOCKS + (ITEMBLOCKS - 1 - i);
            int8_t index3 = (ITEMBLOCKS - 1 - i) * ITEMBLOCKS + (ITEMBLOCKS - 1 - j);
            int8_t index4 = (ITEMBLOCKS - 1 - j) * ITEMBLOCKS + i;

            // Save the current element in a temporary variable
            int8_t temp = mat[index1];

            // Move elements from right column to top row
            mat[index1] = mat[index2];

            // Move elements from bottom row to right column
            mat[index2] = mat[index3];

            // Move elements from left column to bottom row
            mat[index3] = mat[index4];

            // Move elements from top row to left column
            mat[index4] = temp;
        }
    }
    getLeftMargins(ST);
    getRightMargins(ST);
    getBottomMargins(ST);

    // printf("====::  %d   %d\n", checkItemLeftInt(ST, 0), checkItemRightInt(ST, GLASS_W - 1));
    // bool clr = false;
    // while (!clr)
    // {
    //     if (checkItemLeftInt(ST, 0) < 0)
    //     {
    //         ST->x += ST->block_size;
    //         updState(ST);
    //     }
    //     else if (checkItemRightInt(ST, GLASS_W - 1) < 0)
    //     {
    //         ST->x -= ST->block_size;
    //         updState(ST);
    //     }
    //     else
    //     {
    //         clr = true;
    //     }
    // }

    for (int i = 0; i < ITEMBLOCKS; i++)
    {
        if (ST->marg_left[i] != 4 && ST->marg_right[i] != -1)
        {
            printf("%d        %d   %d      %d  %d\n", i, ST->marg_left[i], ST->marg_right[i], checkItemLeft(ST),
                   checkItemRight(ST));
        }
    }
}

/**
 * @brief Print item array values
 *
 * @param ST : State data structure
 */
void printItem(tState *ST)
{
    int8_t *mat = ST->items[ST->ITEM_ID];
    for (int8_t i = 0; i < ITEMBLOCKS; i++)
    {
        for (int8_t j = 0; j < ITEMBLOCKS; j++)
        {
            printf("%d ", mat[i * ITEMBLOCKS + j]);
        }
        printf("\n");
    }
}

/**
 * @brief Copy item's blocks to glass array
 *
 * @param ST : State data structure
 */
void copyBlocksToGlass(tState *ST)
{
    // copy stopped item blocks to glass
    for (int i = 0; i < ITEMBLOCKS; i++)
    {
        for (int j = 0; j < ITEMBLOCKS; j++)
        {
            if (ST->items[ST->ITEM_ID][j * ITEMBLOCKS + i] > 0)
            {
                ST->glass[ST->gy + j][ST->gx + i] = ST->items[ST->ITEM_ID][j * ITEMBLOCKS + i];
            }
        }
    }
}

/**
 * @brief Print glass array values
 *
 * @param ST : State data structure
 */
void printGlass(tState *ST)
{
    for (int i = 0; i < GLASS_H; i++)
    {
        for (int j = 0; j < GLASS_W; j++)
        {
            printf("%d ", ST->glass[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/**
 * @brief Remove specified line, all lined above move down by one line
 *
 * @param ST : State data structure
 * @param line
 */
void removeFullLine(tState *ST, int line)
{
    int fullCells;
    do
    {
        fullCells = 0;
        for (int j = 0; j < GLASS_W; j++)
        {
            ST->glass[line][j] = ST->glass[line - 1][j];
            fullCells += (ST->glass[line][j] != 0);
        }
        --line;
    } while (fullCells != 0 && line > 0);
}

/**
 * @brief Remove full lines from the glass
 *
 * @param ST : State data structure
 * @return true
 * @return false
 */
bool checkRemoveFullLine(tState *ST)
{
    // printGlass(ST);
    int emptyCells;
    for (int i = GLASS_H - 1; i >= 0; i--)
    {
        emptyCells = 0;
        for (int j = 0; j < GLASS_W; j++)
        {
            emptyCells += (ST->glass[i][j] == 0);
        }
        if (emptyCells == 0)
        {
            printf("full line: %d\n", i);

            // remove full line found
            removeFullLine(ST, i);

            return true;
        }
    }
    return false;
}

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
bool parse_opt_arg_uint(int argc, char **argv, int *ind, int *res)
{
    int rc = true;
    int r;

    if (argv[*ind][0] != '-') // non-negative int only
    {
        if (strcmp("0", argv[*ind]) == 0)
        {
            *res = 0;
            *ind += 1;
        }
        else
        {
            r = atoi(argv[*ind]);
            if (r != 0)
            {
                *res = r;
                *ind += 1;
            }
            else
            {
                rc = false;
            }
        }
    }
    else
    {
        rc = false;
    }
    return rc;
}

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
bool parse_opt_arg_str(int argc, char **argv, int *ind, char *res, int len)
{
    int rc = true;

    if (argv[*ind][0] != '-') // must not start with "-"
    {
        strncpy(res, argv[*ind], len);
        *ind += 1;
    }
    else
    {
        rc = 0;
    }
    return rc;
}

/**
 * @brief
 *
 * @param argc : Number of arguments passed to the program
 * @param argv : Values of arguments passed to the program
 * @param conf : Configuration data structure
 * @param DM : Display mode data structure
 */
void parse_args(int argc, char **argv, tConfig *conf, SDL_DisplayMode *DM)
{
    int option;
    extern char *optarg;
    extern int optind;
    char s[10];
    memset(s, 0, sizeof(s));

    while ((option = getopt(argc, argv, "hfirxd")) != -1)
    {
        printf("%d   %s     %c\n", optind, argv[optind - 1], option);
        switch (option)
        {
        case 'd': // -d <w> <h> <{lrtbf}*>
            if (parse_opt_arg_uint(argc, argv, &optind, &conf->w) &&
                parse_opt_arg_uint(argc, argv, &optind, &conf->h) && parse_opt_arg_str(argc, argv, &optind, s, 10))
            {
                printf("option args : %i   %i   %s\n", conf->w, conf->h, s);
                if (strchr(s, 'f'))
                {
                    conf->_f = 1;
                }
                else
                {
                    if (strchr(s, 'l'))
                    {
                        conf->_l = 1;
                    }
                    if (strchr(s, 'r'))
                    {
                        conf->_r = 1;
                    }
                    if (strchr(s, 't'))
                    {
                        conf->_t = 1;
                    }
                    if (strchr(s, 'b'))
                    {
                        conf->_b = 1;
                    }
                }

                // fullscreen (desktop resolution)
                if (conf->_f)
                {
                    conf->x = 0;
                    conf->y = 0;
                    conf->w = DM->w;
                    conf->h = DM->h;
                }
                else
                {
                    // left-right
                    if (conf->_l + conf->_r == 2 || conf->_l + conf->_r == 0)
                    {
                        conf->x = (DM->w - conf->w) / 2;
                    }
                    else if (conf->_l)
                    {
                        conf->x = 0;
                    }
                    else if (conf->_r)
                    {
                        conf->x = DM->w - conf->w;
                    }

                    // top-bottom
                    if (conf->_t + conf->_b == 2 || conf->_t + conf->_b == 0)
                    {
                        conf->y = (DM->h - conf->h) / 2;
                    }
                    else if (conf->_t)
                    {
                        conf->y = 0;
                    }
                    else if (conf->_b)
                    {
                        conf->y = DM->h - conf->h;
                    }
                }
                printf("==  x : %d     y : %d\n", conf->x, conf->y);
            }
            else
            {
                printf("incorrect args for option -%c\n", option);
                exit(EXIT_FAILURE);
            }
            break;
        case 'c':
            printf("option : %c\n", option);
            break;
        case 'f':
            printf("filename : %s\n", optarg);
            break;
        case ':':
            printf("option needs a value\n");
            break;
        case '?':
            printf("unknown option : %c\n", optopt);
            break;
        }
    }
}

/**
 * @brief Draw item
 *
 * @param rend : Renderer data structure
 * @param x : X coordinate of item
 * @param y : Y coordinate of item
 * @param ST : State data structure
 */
void drawItem(SDL_Renderer *rend, int x, int y, tState *ST)
{
    if (ST->GAME_STATE != ITEM_FALLING && ST->GAME_STATE != ITEM_FALLING_FAST)
    {
        return;
    }
    SDL_Rect rect = {x, y, ST->block_size, ST->block_size};
    int e;
    for (int8_t i = 0; i < ITEMBLOCKS; i++)
    {
        for (int8_t j = 0; j < ITEMBLOCKS; j++)
        {
            e = ST->items[ST->ITEM_ID][i * ITEMBLOCKS + j];
            if (e > 0)
            {
                rect.y = y + i * ST->block_size;
                rect.x = x + j * ST->block_size;
                SDL_SetRenderDrawColor(rend, ST->colors[e][0], ST->colors[e][1], ST->colors[e][2], 255);
                SDL_RenderFillRect(rend, &rect);
            }
        }
    }

    rect.w = ST->block_size - 20;
    rect.h = ST->block_size - 20;
    for (int i = 0; i < ITEMBLOCKS; i++)
    {
        if (ST->marg_bottom[i] > -1)
        {
            rect.y = ST->y + ST->marg_bottom[i] * ST->block_size + 10;
            rect.x = ST->x + i * ST->block_size + 10;
            SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
            SDL_RenderFillRect(rend, &rect);
        }
    }
}

/**
 * @brief Draw glass with blocks in it
 *
 * @param rend : Renderer data structure
 * @param ST : State data structure
 */
void drawGlass(SDL_Renderer *rend, tState *ST)
{
    SDL_Rect rect = {ST->glass_x, ST->glass_y, ST->glass_w, ST->glass_h};
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderFillRect(rend, &rect);

    // printf("\n\n");
    for (int i = 0; i < GLASS_H; i++)
    {
        for (int j = 0; j < GLASS_W; j++)
        {
            int e = ST->glass[i][j];
            // printf("%d ", e);
            if (e > 0)
            {
                rect.w = ST->block_size;
                rect.h = ST->block_size;
                rect.y = ST->glass_y + i * ST->block_size;
                rect.x = ST->glass_x + j * ST->block_size;
                SDL_SetRenderDrawColor(rend, ST->colors[e][0], ST->colors[e][1], ST->colors[e][2], 255);
                SDL_RenderFillRect(rend, &rect);
            }
        }
        // printf("\n");
    }
}

/**
 * @brief Initialize glass array
 *
 * @param ST : State data structure
 */
void clearGlass(tState *ST)
{
    for (int i = 0; i < GLASS_H; i++)
    {
        for (int j = 0; j < GLASS_W; j++)
        {
            ST->glass[i][j] = 0;
        }
    }
}

/**
 * @brief Process falling step of item
 *
 * @param ST : State data structure
 */
void fallStep(tState *ST)
{
    if (checkItemBottom(ST))
    {
        copyBlocksToGlass(ST);
        ST->GAME_STATE = ITEM_STOPPED;
    }
    else
    {
        ST->y += ST->block_size;
        updState(ST);
    }
}

/**
 * @brief Main program
 *
 * @param argc : Number of arguments passed to the program
 * @param argv : Values of arguments passed to the program
 * @return int
 */
int main(int argc, char **argv)
{
    tConfig CONF;
    memset(&CONF, 0, sizeof(CONF));

    /* Initializes the timer, audio, video, joystick,
    haptic, gamecontroller and events subsystems */
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 0;
    }

    // Acquire display parameters
    int nDispModes = SDL_GetNumDisplayModes(0);

    SDL_DisplayMode DM;

    for (int i = 0; i < nDispModes; i++)
    {
        SDL_GetDisplayMode(0, i, &DM);
        int w = DM.w;
        int h = DM.h;
        int rr = DM.refresh_rate;
        printf("==   %d         %d   %d   %d\n", i, w, h, rr);
    }

    SDL_GetCurrentDisplayMode(0, &DM);
    printf("==  current    %d   %d   %d\n", DM.w, DM.h, DM.refresh_rate);

    // Parse command line arguments
    parse_args(argc, argv, &CONF, &DM);

    tState ST = {.colors = {{0, 0, 0},      // transparent
                            {228, 26, 28},  // red
                            {255, 255, 51}, // yellow
                            {255, 127, 0},  // orange
                            {77, 175, 74},  // green
                            {152, 78, 163}, // violet
                            {80, 80, 80}},  // gray
                 .items = {{0, 1, 0, 0,     //
                            0, 1, 0, 0,     //
                            0, 1, 0, 0,     //
                            0, 1, 0, 0},    // I
                           {0, 0, 0, 0,     //
                            0, 2, 2, 0,     //
                            0, 2, 2, 0,     //
                            0, 0, 0, 0},    // O
                           {0, 0, 0, 0,     //
                            0, 3, 0, 0,     //
                            0, 3, 0, 0,     //
                            0, 3, 3, 0},    // L
                           {0, 0, 0, 0,     //
                            0, 0, 3, 0,     //
                            0, 0, 3, 0,     //
                            0, 3, 3, 0},    // J
                           {0, 0, 0, 0,     //
                            0, 4, 0, 0,     //
                            0, 4, 4, 0,     //
                            0, 0, 4, 0},    // S
                           {0, 0, 0, 0,     //
                            0, 0, 4, 0,     //
                            0, 4, 4, 0,     //
                            0, 4, 0, 0},    // 4
                           {0, 0, 0, 0,     //
                            0, 5, 0, 0,     //
                            5, 5, 5, 0,     //
                            0, 0, 0, 0}},   // T
                 .fps = 60,
                 .block_size = 25,
                 .GAME_STATE = GAME_WELCOME,
                 .TIMER_FPS = {0, 1000 / ST.fps},
                 .TIMER_1 = {0, 1000},
                 .TIMER_2 = {0, 75},
                 .ITEM_ID = -1};

    /* Create a window */
    uint32_t flags = 0;
    if (CONF._f)
    {
        // flags = flags | SDL_WINDOW_FULLSCREEN_DESKTOP;
        flags = flags | SDL_WINDOW_FULLSCREEN;
    }
    SDL_Window *wind = SDL_CreateWindow("Shell0", CONF.x, CONF.y, CONF.w, CONF.h, flags);
    if (!wind)
    {
        printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    /* Create a renderer */
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Renderer *rend = SDL_CreateRenderer(wind, -1, render_flags);
    if (!rend)
    {
        printf("Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(wind);
        SDL_Quit();
        return 0;
    }
    /* Main loop */
    bool running = true;
    bool put_pressed = false, left_pressed = false, right_pressed = false, up_pressed = false;
    bool put_processed = false, left_processed = false, right_processed = false, up_processed = false;
    SDL_Rect rectScr = {0, 0, CONF.w, CONF.h};
    SDL_Event event;

    ST.glass_w = GLASS_W * ST.block_size;
    ST.glass_h = GLASS_H * ST.block_size;
    ST.glass_x = (CONF.w - ST.glass_w) / 2;
    ST.glass_y = (CONF.h - ST.glass_h) / 2;

    timer_start(&ST.TIMER_FPS);
    timer_start(&ST.TIMER_1);
    timer_start(&ST.TIMER_2);

    while (running)
    {
        /* Process events */
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_Q:
                    running = false;
                    break;
                case SDL_SCANCODE_SPACE:
                    put_pressed = true;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    left_pressed = true;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    right_pressed = true;
                    break;
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    up_pressed = true;
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_SPACE:
                    put_pressed = false;
                    put_processed = false;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    left_pressed = false;
                    left_processed = false;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    right_pressed = false;
                    right_processed = false;
                    break;
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    up_pressed = false;
                    up_processed = false;
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        // Processing
        // TIMER_FPS handling (1000/60 ms)
        if (is_timer_tick(&ST.TIMER_FPS))
        {
            // Process slow timers
            // TIMER_1 handling (1000 ms)
            if (is_timer_tick(&ST.TIMER_1))
            {
                printf("%s\n", getGameState(ST.GAME_STATE));
                switch (ST.GAME_STATE)
                {
                case GAME_WELCOME:
                    break;
                case GAME_STARTED:
                    break;
                case ITEM_STARTED:
                    break;
                case ITEM_FALLING:
                    fallStep(&ST);
                    break;
                case ITEM_FALLING_FAST:
                    break;
                case ITEM_STOPPED:
                    break;
                case GAME_FINISHED:
                    break;
                }

                timer_start(&ST.TIMER_1);
            }

            // TIMER_2 handling (100 ms)
            if (is_timer_tick(&ST.TIMER_2))
            {
                printf("%s\n", getGameState(ST.GAME_STATE));
                switch (ST.GAME_STATE)
                {
                case GAME_WELCOME:
                    break;
                case GAME_STARTED:
                    break;
                case ITEM_STARTED:
                    break;
                case ITEM_FALLING:
                    break;
                case ITEM_FALLING_FAST:
                    fallStep(&ST);
                    break;
                case ITEM_STOPPED:
                    if (!checkRemoveFullLine(&ST))
                    {
                        ST.GAME_STATE = ITEM_STARTED;
                    }
                    break;
                case GAME_FINISHED:
                    break;
                }

                timer_start(&ST.TIMER_2);
            }

            // Process FPS timer
            /* Clear screen */
            SDL_SetRenderDrawColor(rend, ST.colors[6][0], ST.colors[6][1], ST.colors[6][2], 255);
            SDL_RenderFillRect(rend, &rectScr);

            /* Draw glass */
            drawGlass(rend, &ST);

            // printf("%s\n", getGameState(ST.GAME_STATE));
            switch (ST.GAME_STATE)
            {
            case GAME_WELCOME:
                if (put_pressed && !put_processed)
                {
                    ST.GAME_STATE = GAME_STARTED;
                    put_processed = true;
                }
                break;
            case GAME_STARTED:
                clearGlass(&ST);
                ST.GAME_STATE = ITEM_STARTED;
                break;
            case ITEM_STARTED:
                ST.ITEM_ID = rand() % 7;
                ST.x = (CONF.w - ITEMBLOCKS * ST.block_size) / 2;
                ST.y = ITEMBLOCKS * ST.block_size / 2;
                // init item margins
                getLeftMargins(&ST);
                getRightMargins(&ST);
                getBottomMargins(&ST);
                // update state
                updState(&ST);
                drawItem(rend, ST.x, ST.y, &ST);
                SDL_Delay(500);
                ST.GAME_STATE = ITEM_FALLING;
                break;
            case ITEM_FALLING:
            case ITEM_FALLING_FAST:
                if (left_pressed && !left_processed)
                {
                    if (!checkItemLeft(&ST))
                    {
                        ST.x -= ST.block_size;
                        updState(&ST);
                    }
                    left_processed = true;
                }
                if (right_pressed && !right_processed)
                {
                    if (!checkItemRight(&ST))
                    {
                        ST.x += ST.block_size;
                        updState(&ST);
                    }
                    right_processed = true;
                }
                if (up_pressed && !up_processed)
                {
                    rotateItem(&ST);
                    updState(&ST);
                    up_processed = true;
                }
                if (put_pressed && !put_processed)
                {
                    ST.GAME_STATE = ITEM_FALLING_FAST;
                    put_processed = true;
                }
                drawItem(rend, ST.x, ST.y, &ST);
                break;
            case ITEM_STOPPED:
                drawItem(rend, ST.x, ST.y, &ST);
                break;
            case GAME_FINISHED:
                break;
            }

            /* Draw to window and loop */
            SDL_RenderPresent(rend);

            timer_start(&ST.TIMER_FPS);
        }
        else
        {
            SDL_Delay(1);
        }
    }

    /* Release resources */
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(wind);
    SDL_Quit();
    return 0;
}
