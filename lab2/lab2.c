#include <stdio.h>  // for I/O
#include <stdlib.h> // for I/O
#include <libgen.h> // for dirname()/basename()
#include <string.h>

#if 1
    #define LOG(...) do { printf(__VA_ARGS__); } while (0)
#else
    #define LOG(...)
#endif


typedef struct node_t
{
    char name[64]; // node's name string
    char type;     // 'D' for DIR; 'F' for file
    struct node_t* child;
    struct node_t* sibling;
    struct node_t* parent;
} Node;


void init();
int exec_cmd(char* cmd, char* args);
Node* search_child(Node* parent, char* name);
int insert_child(Node* parent, Node* q);

int mkdir(char* pathname);
int rmdir(char* pathname);
int cd(char* pathname);
int ls(char* pathname);
int pwd();
int creat(char* pathname);
int rm(char* pathname);
int save(char* pathname);
int reload(char* pathname);
int menu();
int quit();


const char *CMD_NAMES[] = {
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
    NULL
};

int (*CMD_FUNCS[])(char*) = {
    mkdir,
    rmdir,
    cd,
    ls,
    pwd,
    creat,
    rm,
    save,
    reload,
    menu,
    quit,
};

Node* root;
Node* cwd;

// Node* start;


int main()
{
    init();
    menu();

    char line[128];
    char command[16];
    char args[64];

    while (1)
    {
        printf("Enter command line : ");
        fgets(line, 128, stdin);
        line[strlen(line)-1] = '\0';

        command[0] = args[0] = '\0';
        sscanf(line, "%s %s", command, args);

        printf("command=%s pathname=%s\n", command, args);

        if (command[0] != '\0')
            exec_cmd(command, args);
    }
}


void init()
{
    root = (Node*)malloc(sizeof(Node));
    strcpy(root->name, "/");
    root->parent = root;
    root->sibling = root; // ?
    root->child = NULL;
    root->type = 'D';
    cwd = root;
    printf("Root initialized OK\n");
}

int exec_cmd(char* cmd, char* args)
{
    for (int i = 0; CMD_NAMES[i]; i++)
        if (strcmp(cmd, CMD_NAMES[i]) == 0)
            return CMD_FUNCS[i](args);
    return -1;
}

Node* search_child(Node* parent, char* name)
{
    Node* p;
    LOG("search for %s in parent DIR\n", name);
    p = parent->child;
    while (p)
    {
        if (strcmp(p->name, name) == 0)
            return p;
        p = p->sibling;
    }
    return NULL;
}

int insert_child(Node* parent, Node* q)
{
    Node* p;
    printf("insert NODE %s into END of parent child list\n", q->name);
    p = parent->child;
    if (p == NULL)
        parent->child = q;
    else
    {
        while (p = p->sibling);
        p->sibling = q;
    }
    q->parent = parent;
    q->child = NULL;
    q->sibling = NULL;
}


/***************************************************************
 This mkdir(char *name) makes a DIR in the current directory
 You MUST improve it to mkdir(char *pathname) for ANY pathname
****************************************************************/

int mkdir(char* name)
{
    Node* p;
    Node* q;
    printf("mkdir: name=%s\n", name);

    if (!strcmp(name, "/") || !strcmp(name, ".") || !strcmp(name, ".."))
    {
        printf("can't mkdir with %s\n", name);
        return -1;
    }
    if (name[0] == '/')
        start = root;
    else
        start = cwd;

    printf("check whether %s already exists\n", name);
    p = search_child(start, name);
    if (p)
    {
        printf("name %s already exists, mkdir FAILED\n", name);
        return -1;
    }
    printf("--------------------------------------\n");
    printf("ready to mkdir %s\n", name);
    q = (Node*)malloc(sizeof(Node));
    q->type = 'D';
    strcpy(q->name, name);
    insert_child(start, q);
    printf("mkdir %s OK\n", name);
    printf("--------------------------------------\n");

    return 0;
}

// This ls() list CWD. You MUST improve it to ls(char *pathname)
int ls(char* pathname)
{
    Node* p = cwd->child;
    printf("%s contents = ", pathname);
    while (p)
    {
        printf("[%c %s] ", p->type, p->name);
        p = p->sibling;
    }
    printf("\n");
}

int menu()
{
    printf("commands = [mkdir|rmdir|cd|ls|pwd|creat|rm|save|reload|menu|quit]\n");
}

int quit()
{
    printf("Program exit\n");

    save('filetree.txt');

    exit(0);
    // improve quit() to SAVE the current tree as a Linux file
    // for reload the file to reconstruct the original tree
}
