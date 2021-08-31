#include <stdio.h>
#include <stdlib.h>

int *FP;

int main(int argc, char* argv[], char* env[])
{
    int a, b, c;
    printf("enter main\n");

    printf("&argc=%p argv=%p env=%p\n", &argc, argv, env);
    printf("&a=%p &b=%p &c=%p\n\n", &a, &b, &c);

    // (1). Write C code to print values of argc and argv[] entries
    for (int i = 0; i < argc; i++)
        printf("argv[%d]=%s\n", i, argv[i]);
    printf("\n");
    
    int i = 0;
    while (env[i])
    {
        printf("env[%d]=%s\n", i, env[i]);
        i++;
    }
    printf("\n");

    a=1; b=2; c=3;
    A(a, b);
    printf("exit main\n");
    return 0;
}

int A(int x, int y)
{
    int d, e, f;
    printf("enter A\n");

    // write C code to PRINT ADDRESS OF d, e, f
    printf("&d=%p &e=%p &f=%p\n", &d, &e, &f);

    d=4; e=5; f=6;
    B(d, e);
    printf("exit A\n");
}

int B(int x, int y)
{
    int g, h, i;
    printf("enter B\n");

    // write C code to PRINT ADDRESS OF g,h,i
    printf("&g=%p &h=%p &i=%p\n", &g, &h, &i);

    g=7; h=8; i=9;
    C(g, h);
    printf("exit B\n");
}

int C(int x, int y)
{
    int* p;
    int u, v, w, i;
    printf("enter C\n");

    // write C cdoe to PRINT ADDRESS OF u,v,w,i,p;
    printf("&u=%p &v=%p &w=%p &i=%p &p=%p\n\n", &u, &v, &w, &i, &p);

    u=10; v=11; w=12; i=13;

    FP = (int*)getebp(); // FP = stack frame pointer of the C() function
    // print FP value in HEX
    printf("FP=%p\n", FP);

    // (2). Write C code to print the stack frame link list.
    printf("%p", FP);
    while (FP)
    {
        FP = (int*)(*FP);
        printf(" -> %p", FP);
    }
    printf("\n\n");

    p = (int*)&p;

    // (3). Print the stack contents from p to the frame of main()
    //     YOU MAY JUST PRINT 128 entries of the stack contents.
    for (int j = 0; j < 128; j++)
    {
        printf("%p %8x\n", p, *p);
        p++;
    }
    printf("\n");

    // (4). On a hard copy of the print out, identify the stack contents
    //     as LOCAL VARIABLES, PARAMETERS, stack frame pointer of each function.

    printf("exit C\n");
}
