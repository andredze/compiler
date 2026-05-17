#include <stdio.h>
#include <stdint.h>
#include <math.h>

//------------------------------------------------------------------//

void    ЗаставьИхУслышать(int64_t x);
int64_t СкажиМнеКтоТы    (void);
int64_t Исток            (int64_t x);
void    Нарисуй          (void);
void    Поставь          (int x, int y, char symbol);

//------------------------------------------------------------------//

void ЗаставьИхУслышать(int64_t x)
{
    fprintf(stderr, "%ld\n", x);
}

//------------------------------------------------------------------//

int64_t СкажиМнеКтоТы(void)
{
    int64_t value = 0;

    scanf("%ld", &value);

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

void Нарисуй(void)
{
    for (int y = 0; y < VIDEO_RAM_LENGTH; y++)
    {
        for (int x = 0; x < VIDEO_RAM_LENGTH; x++)
        {
            printf("%c", VIDEO_RAM[y][x]);
        }
    }

    fflush(stdout);
}

//------------------------------------------------------------------//

void Поставь(int x, int y, char symbol)
{
    VIDEO_RAM[y][x] = symbol;
}

//------------------------------------------------------------------//
