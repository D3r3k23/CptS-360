#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>

#define MAX 10000
typedef struct {
    char *name;
    char *value;
} ENTRY;

void my_mkdir(const char* dirname);
void my_rmdir(const char* dirname);
void my_rm(const char* filename);
void my_cat(const char* filename);
void my_cp(const char* file1, const char* file2);
void my_ls(const char* dirname);

void ls_file(const char* filename);
void ls_dir(const char* dirname);

ENTRY entry[MAX];
char cwd[128];

int main(int argc, char *argv[]) 
{
    int n = getinputs(); // get user inputs name=value into entry[ ]
    getcwd(cwd, 128);    // get CWD pathname

    printf("Content-type: text/html\n\n");
    printf("<p>pid=%d uid=%d cwd=%s\n", getpid(), getuid(), cwd);

    printf("<H1>Echo Your Inputs</H1>");
    printf("You submitted the following name/value pairs:<p>");

    for (int i=0; i <= n; i++)
        printf("%s = %s<p>", entry[i].name, entry[i].value);
    printf("<p>");

    /*****************************************************************
     Write YOUR C code here to processs the command
            mkdir dirname
            rmdir dirname
            rm    filename
            cat   filename
            cp    file1 file2
            ls    [dirname] <== ls CWD if no dirname
    *****************************************************************/

    printf("<H1>Output</H1>");

    const char* cmd = entry[0].value;
    const char* f1  = entry[1].value;
    const char* f2  = entry[2].value;

    if (strcmp(cmd, "mkdir") == 0)
        my_mkdir(f1);
    else if (strcmp(cmd, "rmdir") == 0)
        my_rmdir(f1);
    else if (strcmp(cmd, "rm") == 0)
        my_rm(f1);
    else if (strcmp(cmd, "cat") == 0)
        my_cat(f1);
    else if (strcmp(cmd, "cp") == 0)
        my_cp(f1, f2);
    else if (strcmp(cmd, "ls") == 0)
        my_ls(f1);
    else
        printf("command: \"%s\" is not a valid command\n", cmd);

    printf("<p>");

    // create a FORM webpage for user to submit again 
    printf("</title>");
    printf("</head>");
    printf("<body bgcolor=\"#FF0000\" link=\"#330033\" leftmargin=8 topmargin=8");
    printf("<p>------------------ DO IT AGAIN ----------------");

    //------ NOTE : CHANGE ACTION to YOUR login name ----------------------------
    printf("<FORM METHOD=\"POST\" ACTION=\"http://69.166.48.15/~henderson/cgi-bin/mycgi.bin\">");

    printf("Enter command : <INPUT NAME=\"command\"> <P>");
    printf("Enter filename1: <INPUT NAME=\"filename1\"> <P>");
    printf("Enter filename2: <INPUT NAME=\"filename2\"> <P>");
    printf("Submit command: <INPUT TYPE=\"submit\" VALUE=\"Click to Submit\"><P>");
    printf("</form>");
    printf("------------------------------------------------<p>");

    printf("</body>");
    printf("</html>");

    return 0;
}

void my_mkdir(const char* dirname)
{
    mkdir(dirname, 0766);
}

void my_rmdir(const char* dirname)
{
    rmdir(dirname);
}

void my_rm(const char* filename)
{
    unlink(filename);
}

void my_cat(const char* filename)
{
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

void my_cp(const char* file1, const char* file2)
{
    struct stat sp;
    lstat(file1, &sp);
    mode_t mode = sp.st_mode;

    int srcFD, destFD;
    if ((srcFD = open(file1, O_RDONLY)) < 0)
        printf("Could not open %s<p>", file1);
    else if ((destFD = open(file2, O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0)
        printf("Could not open %s<p>", file2);
    else
    {
        char buf[4096];
        ssize_t n;
        while (n = read(srcFD, buf, 4096))
            write(destFD, buf, n);

        close(srcFD);
        close(destFD);
    }
}

void my_ls(const char* pathname)
{
    if (!pathname || strlen(pathname) == 0)
        pathname = cwd;
    
    struct stat sp;
    lstat(pathname, &sp);

    if (S_ISREG(sp.st_mode))
        ls_file(pathname);
    else if (S_ISDIR(sp.st_mode))
        ls_dir(pathname);
}

void ls_file(const char* filename)
{
    static const char* modes = "rwxrwxrwx";

    struct stat sp;
    lstat(filename, &sp);

    if (S_ISREG(sp.st_mode))
        printf("%c", '-');
    if (S_ISDIR(sp.st_mode))
        printf("%c", 'd');
    if (S_ISLNK(sp.st_mode))
        printf("%c", 'l');

    for (int i = 0; i < 9; i++)
        if (sp.st_mode & (0x1 << (8 - i)))
            printf("%c", modes[i]);
        else
            printf("%c", '-');

    printf("%4d ", sp.st_nlink);
    printf("%4d ", sp.st_gid);
    printf("%4d ", sp.st_uid);
    printf("%4d ", sp.st_size);

    printf("%s ", ctime(&sp.st_ctime));

    char tmp[128];
    strcpy(tmp, filename);
    printf("%s", basename(tmp));

    if (S_ISLNK(sp.st_mode))
    {
        char linkname[128];
        readlink(filename, linkname, 128);
        linkname[127] = '\0';
        printf(" -> %s", linkname);
    }
    printf("<p>");
}

void ls_dir(const char* dirname)
{
    DIR* dir = opendir(dirname);
    if (!dir)
        printf("Unable to open directory \"%s\"", dirname);
    else
    {
        struct dirent* ent;
        while (ent = readdir(dir))
        {
            if (ent->d_name[0] == '.')
                continue;
            if (ent->d_name[strlen(ent->d_name) - 1] == '~')
                continue;

            ls_file(ent->d_name);
        }
    }
    closedir(dir);
}
