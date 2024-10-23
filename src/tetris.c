#include "tetris.h"

void timer_start(tTimer *t)
{
    t->last = GetTickCount();
}
DWORD timer_check(tTimer *t)
{
    return GetTickCount() - t->last;
}
bool is_timer_tick(tTimer *t)
{
    return (GetTickCount() - t->last) >= t->ms;
}

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

void updState(tState *ST)
{
    ST->gx = (ST->x - ST->glass_x) / ST->block_size;
    ST->gy = (ST->y - ST->glass_y) / ST->block_size;
}

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

