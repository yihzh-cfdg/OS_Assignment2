#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
int main(void)
{
    int   x = 5;
    if (fork())
    {
        x += 30;
        printf("%d\n", x);
    }
    else
        printf("%d\n", x);
    printf("%d\n", x);
    return 0;
}
