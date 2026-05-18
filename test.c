// #include <stdio.h>

// int Факториал()
// {
// 	volatile int b;
// 	return b;
// }

// int Квадратка()
// {
// 	volatile int b;
// 	return b;
// }

// int Линейка()
// {
// 	volatile int b;
// 	return b;
// }

// int Жопа()
// {
// 	int a;
// 	scanf("%d", &a);
// 	return 2;
// }

// int main()
// {
// 	printf("hi sashsa");

// 	asm volatile("mov rax, rbx" : : :);
// 	asm volatile("mov rbp, r15" : : :);
// 	asm volatile("mov r10, r10" : : :);
// 	asm volatile("mov rax, [rsp]" : : :);
// 	asm volatile("mov r11, [rbp]" : : :);
// 	asm volatile("mov rax, [rdx + 0xDED]" : : :);
// 	asm volatile("mov rax, [rsp + 1000]" : : :);
// 	asm volatile("mov r11, [rbp - 1000]" : : :);
// 	asm volatile("add r13, rax" : : :);
// 	asm volatile("sub r13, rax" : : :);

// 	return Жопа();
// }
#include <stdio.h>

int скажи_мне_кто_ты()
{
	int a = 0;
	scanf("%d", &a);
	return a;
}

int func()
{
	return 0;
}

#include "lib.c"

int main(int args, char* argv[])
{
    int radius = 0;
    
    int x_0 = VIDEO_RAM_LENGTH / 2;
    int y_0 = VIDEO_RAM_LENGTH / 2;

    if (args > 1)
    {
        sscanf(argv[1], "%d", &radius);
    }

    fprintf(stderr, "radius %d\n", radius);

    for (int y = 0; y < VIDEO_RAM_LENGTH; y++)
    {
        for (int x = 0; x < VIDEO_RAM_LENGTH; x++)
        {
            fprintf(stderr, "              y     x %d %d\n", y, x);
            fprintf(stderr, "from center y-y0 x-x0 %d %d\n", y-y_0, x-x_0);
            fprintf(stderr, "squared     y-y0 x-x0 %d %d\n", (y-y_0) * (y-y_0), (x-x_0) * (x-x_0));
            fprintf(stderr, "            r_squared %d\n"   , radius * radius);
            fprintf(stderr, "            condition %d\n"   , ((x - x_0) * (x - x_0) + (y - y_0) * (y - y_0)) <= radius * radius);

            if (((x - x_0) * (x - x_0) + (y - y_0) * (y - y_0)) <= radius * radius)
            {
                Поставь(x, y, '@');
            }
        }
    }

    Нарисуй();

    return 0;
}

//------------------------------------------------------------------//
