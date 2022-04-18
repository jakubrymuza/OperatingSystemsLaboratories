#include <stdio.h>
#include <stdlib.h>
#define HEIGHT 4
#define WIDTH 4

int main(int argc, char **argv)
{
    int tab[HEIGHT*WIDTH], add, addA, i;
    tab[0] = add = addA = i = 1;

    while(i<HEIGHT*WIDTH)
    {
        for(int j = 0; j<addA; ++j)
        {
            tab[i] = tab[i-1]+add;
            ++i;
        }

        if(i<HEIGHT*WIDTH/2)
        {
            ++add;
            addA+=2;
        }
        else 
        {
            --add;
            addA-=2;
        }

        
    }

    for(int i =0; i<HEIGHT*WIDTH; ++i)
        printf("%d ", tab[i]);

    return EXIT_SUCCESS;
}

