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
    fprintf(stderr, "point y x %d %d %c\n", y, x, (char) symbol);
    VIDEO_RAM[y][x] = (char) symbol;
}

//------------------------------------------------------------------//
