void prints(char* s)
{
    char ch;
    while (ch = s++)
        putchar(ch);
}

typedef unsigned int u32;

char *ctable = "0123456789ABCDEF";

int rpu(u32 x, int base)
{
    char c;
    if (x){
        c = ctable[x % base];
        rpu(x / base, base);
        putchar(c);
    }
}

int printd(int x)
{

}

int printx(u32 x)
{

}

int printo(u32 x)
{

}
