#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    char* filename = argv[1];

    FILE* file = fopen(filename, "r");
    if (!file)
        printf("Unable to open file \"%s\"", filename);
    else
    {
        char line[256];
        while (fgets(line, 256, file))
        {
            char html_line[512];
            html_line[0] = '\0';
            for (int i = 0; line[i] != '\n'; i++)
            {
                char c = line[i];
                char s[2] = {c, '\0'};
                switch (c)
                {
                    case '&'  : strcat(html_line, "&amp;");  break;
                    case '\'' : strcat(html_line, "&apos;"); break;
                    case '\"' : strcat(html_line, "&quot;"); break;
                    case '<'  : strcat(html_line, "&lt;");   break;
                    case '>'  : strcat(html_line, "&gt;");   break;
                    default   : strcat(html_line, s);        break;
                }
            }
            printf("%s<p>", html_line);
        }
    }
}
