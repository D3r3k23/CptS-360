#include <stdio.h>  // for I/O
#include <stdlib.h> // for I/O
#include <libgen.h> // for dirname()/basename()
#include <string.h>

typedef struct node_t
{
    char name[64]; // node's name string
    char type;     // 'D' for DIR; 'F' for file
    struct node_t* child;
    struct node_t* sibling;
    struct node_t* parent;
} NODE;

typedef enum cmd_t
{
    MKDIR,
    RMDIR,
    CD,
    LS,
    PWD,
    CREAT,
    RM,
    SAVE,
    RELOAD,
    MENU,
    QUIT,
    NONE
} CMD;


NODE* root;
NODE* cwd;
NODE* start;

char line[128];
char command[16];
char pathname[64];


CMD findCmd(char *command)
{
    static const char *cmds[] = {
        "mkdir",
        "rmdir",
        "cd",
        "ls",
        "pwd",
        "creat",
        "rm",
        "save",
        "reload",
        "menu",
        "quit",
        0
    };

    for (int i = 0; cmds[i]; i++)
        if (strcmp(command, cmds[i]) == 0)
            return (CMD)i;
            
    return NONE;
}

NODE *search_child(NODE *parent, char *name)
{
  NODE *p;
  printf("search for %s in parent DIR\n", name);
  p = parent->child;
  if (p==0)
    return 0;
  while(p){
    if (strcmp(p->name, name)==0)
      return p;
    p = p->sibling;
  }
  return 0;
}

int insert_child(NODE *parent, NODE *q)
{
  NODE *p;
  printf("insert NODE %s into END of parent child list\n", q->name);
  p = parent->child;
  if (p==0)
    parent->child = q;
  else{
    while(p->sibling)
      p = p->sibling;
    p->sibling = q;
  }
  q->parent = parent;
  q->child = 0;
  q->sibling = 0;
}

/***************************************************************
 This mkdir(char *name) makes a DIR in the current directory
 You MUST improve it to mkdir(char *pathname) for ANY pathname
****************************************************************/

int mkdir(char *name)
{
  NODE *p, *q;
  printf("mkdir: name=%s\n", name);

  if (!strcmp(name, "/") || !strcmp(name, ".") || !strcmp(name, "..")){
    printf("can't mkdir with %s\n", name);
    return -1;
  }
  if (name[0]=='/')
    start = root;
  else
    start = cwd;

  printf("check whether %s already exists\n", name);
  p = search_child(start, name);
  if (p){
    printf("name %s already exists, mkdir FAILED\n", name);
    return -1;
  }
  printf("--------------------------------------\n");
  printf("ready to mkdir %s\n", name);
  q = (NODE *)malloc(sizeof(NODE));
  q->type = 'D';
  strcpy(q->name, name);
  insert_child(start, q);
  printf("mkdir %s OK\n", name);
  printf("--------------------------------------\n");
    
  return 0;
}

// This ls() list CWD. You MUST improve it to ls(char *pathname)
void ls(char* pathname)
{
    NODE* p = cwd->child;
    printf("%s contents = ", pathname);
    while (p)
    {
        printf("[%c %s] ", p->type, p->name);
        p = p->sibling;
    }
    printf("\n");
}

void quit()
{
    printf("Program exit\n");




    exit(0);
    // improve quit() to SAVE the current tree as a Linux file
    // for reload the file to reconstruct the original tree
}

void menu()
{
    printf("commands = [mkdir|rmdir|cd|ls|pwd|creat|rm|save|reload|menu|quit]\n");
}

void initialize()
{
    root = (NODE*)malloc(sizeof(NODE));
    strcpy(root->name, "/");
    root->parent = root;
    root->sibling = 0;
    root->child = 0;
    root->type = 'D';
    cwd = root;
    printf("Root initialized OK\n");
}

int main()
{
    initialize();
    menu();

    while (1)
    {
        printf("Enter command line : ");
        fgets(line, 128, stdin);
        line[strlen(line)-1] = 0;

        command[0] = pathname[0] = 0;
        sscanf(line, "%s %s", command, pathname);
        printf("command=%s pathname=%s\n", command, pathname);

        if (command[0] == 0) 
            continue;

        CMD cmd = findCmd(command);
        switch (cmd)
        {
            case MKDIR  : mkdir(pathname);  break;
            case RMDIR  : rmdir(pathname);  break;
            case CD     : cd(pathname);     break;
            case LS     : ls(pathname);     break;
            case PWD    : pwd();            break;
            case CREAT  : creat(pathname);  break;
            case RM     : rm(pathname);     break;
            case SAVE   : save(pathname);   break;
            case RELOAD : reload(pathname); break;
            case MENU   : menu();           break;
            case QUIT   : quit();           break;
        }
    }
}

