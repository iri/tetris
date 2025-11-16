#include "tetris.h"

void timer_start(tTimer* t)
{
    t->last = GetTickCount();
}

DWORD timer_check(tTimer* t)
{
    return GetTickCount() - t->last;
}

bool is_timer_tick(tTimer* t)
{
    return (GetTickCount() - t->last) >= t->ms;
}

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

void updState(tState* ST)
{
    ST->gx = (ST->x - ST->glass_x) / ST->block_size;
    ST->gy = (ST->y - ST->glass_y) / ST->block_size;
}

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
}

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
}

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

void drawWelcomeScreen(SDL_Renderer* rend, tState* ST)
{
    // Try multiple possible font paths
    const char* font_paths[] = {
        FONT_PATH,
        "src/" FONT_PATH,
        "../src/" FONT_PATH,
        NULL
    };
    
    TTF_Font* title_font = NULL;
    TTF_Font* subtitle_font = NULL;
    
    // Try to load font from different possible locations
    for (int i = 0; font_paths[i] != NULL; i++)
    {
        title_font = TTF_OpenFont(font_paths[i], TITLE_FONT_SIZE);
        subtitle_font = TTF_OpenFont(font_paths[i], SUBTITLE_FONT_SIZE);
        if (title_font && subtitle_font)
        {
            break; // Successfully loaded
        }
        // Clean up if partially loaded
        if (title_font) TTF_CloseFont(title_font);
        if (subtitle_font) TTF_CloseFont(subtitle_font);
        title_font = NULL;
        subtitle_font = NULL;
    }
    
    if (!title_font || !subtitle_font)
    {
        // If font loading fails, just return (game will still work without text)
        if (title_font) TTF_CloseFont(title_font);
        if (subtitle_font) TTF_CloseFont(subtitle_font);
        return;
    }
    
    SDL_Color text_color = {255, 255, 255, 255}; // White color
    
    // Render title "TETRIS"
    SDL_Surface* title_surface = TTF_RenderText_Solid(title_font, "TETRIS", text_color);
    if (title_surface)
    {
        SDL_Texture* title_texture = SDL_CreateTextureFromSurface(rend, title_surface);
        if (title_texture)
        {
            int title_w, title_h;
            SDL_QueryTexture(title_texture, NULL, NULL, &title_w, &title_h);
            SDL_Rect title_rect = {(1200 - title_w) / 2, 250, title_w, title_h};
            SDL_RenderCopy(rend, title_texture, NULL, &title_rect);
            SDL_DestroyTexture(title_texture);
        }
        SDL_FreeSurface(title_surface);
    }
    
    // Render subtitle "Press SPACE to start"
    SDL_Surface* subtitle_surface = TTF_RenderText_Solid(subtitle_font, "Press SPACE to start", text_color);
    if (subtitle_surface)
    {
        SDL_Texture* subtitle_texture = SDL_CreateTextureFromSurface(rend, subtitle_surface);
        if (subtitle_texture)
        {
            int subtitle_w, subtitle_h;
            SDL_QueryTexture(subtitle_texture, NULL, NULL, &subtitle_w, &subtitle_h);
            SDL_Rect subtitle_rect = {(1200 - subtitle_w) / 2, 400, subtitle_w, subtitle_h};
            SDL_RenderCopy(rend, subtitle_texture, NULL, &subtitle_rect);
            SDL_DestroyTexture(subtitle_texture);
        }
        SDL_FreeSurface(subtitle_surface);
    }
    
    TTF_CloseFont(title_font);
    TTF_CloseFont(subtitle_font);
}

