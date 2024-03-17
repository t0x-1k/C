#include <stdio.h>
#include <ctype.h>
#include <string.h>


int main(void)
{
    char input; 
    printf("Enter an character: ");
    scanf("%c", &input);

    printf("You entered: %c", input);
    
    return 0;
}