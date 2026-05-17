#include <stdio.h>
#include <stdint.h>
#include <math.h>

//------------------------------------------------------------------//

int ЗаставьИхУслышать(int x);
int СкажиМнеКтоТы    (void);
int Исток            (int x);

//------------------------------------------------------------------//

int ЗаставьИхУслышать(int x)
{
    fprintf(stderr, "%d\n", x);
}

//------------------------------------------------------------------//

int СкажиМнеКтоТы(void)
{
    int value = 0;

    scanf("%d", &value);

    return value;
}

//------------------------------------------------------------------//

int Исток(int x)
{
    return (int) sqrt(x);
}

//------------------------------------------------------------------//

#define VIDEO_RAM_LENGTH 50
#define VIDEO_RAM_SIZE   VIDEO_RAM_LENGTH * VIDEO_RAM_LENGTH

static char VIDEO_RAM[VIDEO_RAM_LENGTH][VIDEO_RAM_LENGTH] = {};

//------------------------------------------------------------------//

int Нарисуй()
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

int Поставь(int x, int y, char symbol)
{
    VIDEO_RAM[y][x] = symbol;
}

//------------------------------------------------------------------//
