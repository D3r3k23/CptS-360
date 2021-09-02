#include <stdio.h>

typedef unsigned int u32;

void prints(char* s)
{
    char ch;
    while (ch = *(s++))
        putchar(ch);
}

void rpu(u32 x, int base)
{
    static const char *ctable = "0123456789ABCDEF";

    char c;
    if (x)
    {
        c = ctable[x % base];
        rpu(x / base, base);
        putchar(c);
    }
}

void printu(u32 x)
{
    if (x == 0)
        putchar('0');
    else
        rpu(x, 10);
}

void printd(int x)
{
    if (x < 0)
    {
        putchar('-');
        x = -x;
    }
    printu(x);
}

void printx(u32 x)
{
    prints("0x");
    if (x == 0)
        putchar('0');
    else
        rpu(x, 16);
}

void printo(u32 x)
{
    putchar('0');
    if (x == 0)
        putchar('0');
    else
        rpu(x, 8);
}

void printb(u32 x)
{
    prints("0b");
    if (x == 0)
        putchar('0');
    else
        rpu(x, 2);
}

void myprintf(char* fmt, ...)
{
    int* arg_ptr = (int*)&fmt + 1;

    int i = 0;
    char c;
    while(c = fmt[i++])
    {
        if (c == '%')
        {
            char fmt_char = fmt[i++]; // Next char after '%' (skip for next iteration)
            switch (fmt_char)
            {
                case 'c': putchar((char)(*arg_ptr)); break;
                case 's': prints((char*)(*arg_ptr)); break;
                case 'u': printu((u32)(*arg_ptr));   break;
                case 'd': printd((int)(*arg_ptr));   break;
                case 'o': printo((u32)(*arg_ptr));   break;
                case 'x': printx((u32)(*arg_ptr));   break;
                case 'b': printb((u32)(*arg_ptr));   break;
                default: break;
            }
            arg_ptr++;
        }
        else if (c == '\n')
            prints("\n\r");
        else
            putchar(c);
    }
}

int main(int argc, char* argv[], char* env[])
{
    myprintf("char: %c\n", 'd');
    myprintf("string: %s\n", "test");
    myprintf("u32: %u\n", 23u);
    myprintf("int: %d\n", -8);
    myprintf("octal: %o\n", 15);
    myprintf("hex: %x\n", 19);
    myprintf("bin: %b\n", 10);
    myprintf("\n");

    myprintf("argc = %d\n", argc);
    myprintf("\n");

    for (int i = 0; argv[i]; i++)
        myprintf("argv[%d] = %s\n", i, argv[i]);
    myprintf("\n");

    for (int i = 0; env[i]; i++)
        myprintf("env[%d] = %s\n", i, env[i]);

    return 0;
}
