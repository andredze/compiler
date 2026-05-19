#include <stdio.h>
#include <stdint.h>
#include <math.h>

//------------------------------------------------------------------//

void    ЗаставьИхУслышать(int64_t x);
int64_t СкажиМнеКтоТы    (void);
int64_t Исток            (int64_t x);
void    Нарисуй          (void);
void    Поставь          (int x, int y, int symbol);

//------------------------------------------------------------------//

void ЗаставьИхУслышать(int64_t x)
{
    fprintf(stderr, "%ld\n", x);
}

//------------------------------------------------------------------//

const int64_t ERROR_POISON = 0xDEAFBABA;

//------------------------------------------------------------------//

int64_t СкажиМнеКтоТы(void)
{
    int64_t value = 0;

    if (scanf("%ld", &value) != 1)
    {
        return ERROR_POISON;
    }

    return value;
}

//------------------------------------------------------------------//

int64_t Исток(int64_t x)
{
    return (int64_t) sqrt(x);
}

//------------------------------------------------------------------//

#define VIDEO_RAM_LENGTH 50
#define VIDEO_RAM_SIZE   VIDEO_RAM_LENGTH * VIDEO_RAM_LENGTH

static char VIDEO_RAM[VIDEO_RAM_LENGTH][VIDEO_RAM_LENGTH] = {};

//------------------------------------------------------------------//

static void ClearVram(void)
{
    for (int y = 0; y < VIDEO_RAM_LENGTH; y++)
    {
        for (int x = 0; x < VIDEO_RAM_LENGTH; x++)
        {
            VIDEO_RAM[y][x] = 0;
        }
    }
}

//------------------------------------------------------------------//

void Нарисуй(void)
{
    for (int y = 0; y < VIDEO_RAM_LENGTH; y++)
    {
        for (int x = 0; x < VIDEO_RAM_LENGTH; x++)
        {
            if (VIDEO_RAM[y][x] == 0)
            {
                printf(" ");
            }
            else
            {
                printf("%c", VIDEO_RAM[y][x]);
            }
            printf(" ");
        }
        
        printf("\n");
    }

    fflush(stdout);

    ClearVram();
}

//------------------------------------------------------------------//

void Поставь(int x, int y, int symbol)
{

#ifdef DEBUG
    fprintf(stderr, "point y x %d %d %c\n", y, x, (char) symbol);
#endif /* DEBUG */

    VIDEO_RAM[y][x] = (char) symbol;
}

//------------------------------------------------------------------//

#include <SDL3/SDL.h>

const char * const APP_WINDOW_NAME = "приложуха";

static SDL_Surface* screen_surface = NULL;
static SDL_Window*  window         = NULL;

//------------------------------------------------------------------/

int ИнициализируйЭкран(int window_width, int window_height)
{
    //------------------------------------------------------------------//

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    //------------------------------------------------------------------//

    window = SDL_CreateWindow(APP_WINDOW_NAME,
                              window_width,
                              window_height,
                              0);
    if (window == NULL)
    {
        fprintf(stderr, "Failed creating window. SDL_Error: %s", SDL_GetError());
        return 1;
    }

    //------------------------------------------------------------------//

    screen_surface = SDL_GetWindowSurface(window);

    if (screen_surface == NULL)
    {
        fprintf(stderr, "Failed getting window surface. SDL_Error: %s", SDL_GetError());
        return 1;
    }

    //------------------------------------------------------------------//

    return 0;
}

//------------------------------------------------------------------//

int ПоставьНаЭкран(int x, int y, int r, int g, int b, int a)
{
    if (!SDL_WriteSurfacePixel(screen_surface, x, y, r, g, b, a))
    {
        fprintf(stderr, "Failed writing surface pixel. SDL_Error: %s", SDL_GetError());
        return 1;
    }

    return 0;
}

//------------------------------------------------------------------//

int НарисуйЭкран()
{
    if (!SDL_UpdateWindowSurface(window))
    {
        fprintf(stderr, "Failed updating window surface. SDL_Error: %s", SDL_GetError());
        return 1;
    }

    SDL_Delay(10000);

    return 0;
}

//------------------------------------------------------------------//

void УдалиЭкран()
{
    SDL_DestroyWindowSurface(window);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

//------------------------------------------------------------------//