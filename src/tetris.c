#include "tetris.h"

/**
 * @brief Starts a timer by recording the current tick count
 * @param t Pointer to the timer structure to initialize
 */
void timer_start(tTimer* t)
{
    t->last = GetTickCount();
}

/**
 * @brief Checks how many milliseconds have elapsed since the timer started
 * @param t Pointer to the timer structure
 * @return Number of milliseconds elapsed
 */
DWORD timer_check(tTimer* t)
{
    return GetTickCount() - t->last;
}

/**
 * @brief Checks if the timer has reached its specified interval
 * @param t Pointer to the timer structure
 * @return true if the timer interval has elapsed, false otherwise
 */
bool is_timer_tick(tTimer* t)
{
    return (GetTickCount() - t->last) >= t->ms;
}

/**
 * @brief Converts a game state enum to its string representation
 * @param state The game state to convert
 * @return String representation of the game state
 */
const char* getGameState(tGameState state)
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
 * @brief Finds the minimum value among four integers
 * @param a First number
 * @param b Second number
 * @param c Third number
 * @param d Fourth number
 * @return The minimum value among the four numbers
 */
int8_t min4(int8_t a, int8_t b, int8_t c, int8_t d)
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
 * @brief Finds the maximum value among four integers
 * @param a First number
 * @param b Second number
 * @param c Third number
 * @param d Fourth number
 * @return The maximum value among the four numbers
 */
int8_t max4(int8_t a, int8_t b, int8_t c, int8_t d)
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
 * @brief Updates the grid coordinates based on current position
 * @param ST Pointer to the game state structure
 */
void updState(tState* ST)
{
    ST->gx = (ST->x - ST->glass_x) / ST->block_size;
    ST->gy = (ST->y - ST->glass_y) / ST->block_size;
}

/**
 * @brief Calculates the bottom margins for each column of the current tetromino
 * @param ST Pointer to the game state structure
 */
void getBottomMargins(tState* ST)
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
 * @brief Calculates the left margins for each row of the current tetromino
 * @param ST Pointer to the game state structure
 */
void getLeftMargins(tState* ST)
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
 * @brief Calculates the right margins for each row of the current tetromino
 * @param ST Pointer to the game state structure
 */
void getRightMargins(tState* ST)
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
 * @brief Checks if the current tetromino can move left
 * @param ST Pointer to the game state structure
 * @return true if movement is blocked, false if movement is possible
 */
bool checkItemLeft(tState* ST)
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
 * @brief Checks if the current tetromino can move right
 * @param ST Pointer to the game state structure
 * @return true if movement is blocked, false if movement is possible
 */
bool checkItemRight(tState* ST)
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

/**
 * @brief Checks if the current tetromino can move down
 * @param ST Pointer to the game state structure
 * @return true if movement is blocked, false if movement is possible
 */
bool checkItemBottom(tState* ST)
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
 * @brief Rotates the current tetromino 90 degrees clockwise
 * @param ST Pointer to the game state structure
 */
void rotateItem(tState* ST)
{
    int8_t* mat = ST->items[ST->ITEM_ID];
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
 * @brief Prints the current tetromino matrix for debugging
 * @param ST Pointer to the game state structure
 */
void printItem(tState* ST)
{
    int8_t* mat = ST->items[ST->ITEM_ID];
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
 * @brief Copies the current tetromino blocks to the game glass
 * @param ST Pointer to the game state structure
 */
void copyBlocksToGlass(tState* ST)
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
 * @brief Prints the current state of the game glass for debugging
 * @param ST Pointer to the game state structure
 */
void printGlass(tState* ST)
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
 * @brief Removes a full line and shifts all lines above it down
 * @param ST Pointer to the game state structure
 * @param line The line number to remove
 */
void removeFullLine(tState* ST, int line)
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
 * @brief Checks for and removes any full lines in the game glass
 * @param ST Pointer to the game state structure
 * @return true if a line was removed, false otherwise
 */
bool checkRemoveFullLine(tState* ST)
{
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
 * @brief Draws the current tetromino on the screen
 * @param rend SDL renderer pointer
 * @param x X coordinate for drawing
 * @param y Y coordinate for drawing
 * @param ST Pointer to the game state structure
 */
void drawItem(SDL_Renderer* rend, int x, int y, tState* ST)
{
    if (ST->GAME_STATE != ITEM_FALLING && ST->GAME_STATE != ITEM_FALLING_FAST)
    {
        return;
    }
    SDL_Rect rect = { x, y, ST->block_size, ST->block_size };
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
 * @brief Draws the game glass and all placed blocks
 * @param rend SDL renderer pointer
 * @param ST Pointer to the game state structure
 */
void drawGlass(SDL_Renderer* rend, tState* ST)
{
    SDL_Rect rect = { ST->glass_x, ST->glass_y, ST->glass_w, ST->glass_h };
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderFillRect(rend, &rect);

    for (int i = 0; i < GLASS_H; i++)
    {
        for (int j = 0; j < GLASS_W; j++)
        {
            int e = ST->glass[i][j];
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
    }
}

/**
 * @brief Clears all blocks from the game glass
 * @param ST Pointer to the game state structure
 */
void clearGlass(tState* ST)
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
 * @brief Handles one step of tetromino falling movement
 * @param ST Pointer to the game state structure
 */
void fallStep(tState* ST)
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

