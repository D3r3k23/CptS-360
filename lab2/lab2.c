#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>


#ifdef LOG // gcc -DLOG to enable
    #undef LOG
    // Forward __VA_ARGS__ to printf
    #define LOG(...)                             \
        do {                                     \
            printf("[LOG] <%s> ", __FUNCTION__); \
            printf(__VA_ARGS__);                 \
            printf("\n");                        \
        } while (0)
#else
    #define LOG(...)
#endif


////////// File system tree node //////////
typedef struct node_t
{
    char name[64];
    char type;     // D for dir | F for file
    struct node_t* child;
    struct node_t* sibling;
    struct node_t* parent;
} Node;


////////// Program flow //////////
void init(void);
int exec_cmd(char* cmd, char* args);

////////// File system tree operations //////////
Node* make_node(char* name, char type);
int delete_node(Node* node);
Node* search_children(Node* parent, char* name);
int insert_child(Node* parent, Node* q);

////////// Path operations //////////
int split_path(char* path, char names_o[][16]);
Node* path2node(char* path);
void node2path(Node* node, char* path_o);

////////// Command helpers //////////
int create_item(char type, char* name);
int remove_item(char type, char* path);
void write_tree(FILE* fp, Node* node);

////////// Commands //////////
int mkdir(char* path);
int rmdir(char* path);
int cd(char* path);
int ls(char* path);
int pwd();
int creat(char* path);
int rm(char* path);
int save(char* fn);
int reload(char* fn);
int menu();
int tree();
int reset();
int quit();


////////// Globals //////////

char *CMD_NAMES[] = {
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
    "tree",
    "reset",
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
    tree,
    reset,
    quit,
};

Node* root = NULL;
Node* cwd  = NULL;

/**************************************/
/**************** Main ****************/
/**************************************/
int main()
{
    init();
    printf("======== File system tree ========\n");
    printf("Menu:\n");
    menu();

    char line[128];
    char command[16];
    char args[128];

    while (1)
    {
        printf("FS: [%s]$ ", cwd->name);
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = '\0';

        command[0] = args[0] = '\0';
        sscanf(line, "%s %s", command, args);

        if (command[0] != '\0')
            exec_cmd(command, args);
    }
}


void init(void)
{
    cwd = root = make_node("/", 'D');
    LOG("Root initialized");
}

int exec_cmd(char* cmd, char* args)
{
    for (int i = 0; CMD_NAMES[i]; i++)
        if (strcmp(cmd, CMD_NAMES[i]) == 0)
        {
            LOG("Executing command: %s %s", cmd, args);
            return CMD_FUNCS[i](args);
        }
    printf("Error: Command %s not found. Enter \"menu\" to view a list of commands\n", cmd);
    return -1;
}

Node* make_node(char* name, char type)
{
    Node* node = (Node*)malloc(sizeof(Node));
    strcpy(node->name, name);
    node->type    = type;
    node->child   = NULL;
    node->sibling = NULL;
    node->parent  = NULL;
}

int delete_node(Node* node)
{
    if (!node)
        return 0;

    if (!node->parent)
    {
        printf("Error: Cannot delete the root node\n");
        return -1;
    }

    // Remove node from tree
    Node* parent = node->parent;
    if (parent->child == node) // Node is first child in parent's linked list
    {
        parent->child = node->sibling;
    }
    else
    {
        Node* prev_sibling = parent->child;
        while (prev_sibling->sibling != node) // Find node's previous sibling in linked list
            prev_sibling = prev_sibling->sibling;
        prev_sibling->sibling = node->sibling;
    }

    // Delete node's subtree
    while (node->child)
        delete_node(node->child);
    
    free(node);
    return 0;
}

Node* search_children(Node* parent, char* name)
{
    LOG("Searching for child node %s in parent node %s", name, parent->name);
    Node* node = parent->child;
    while (node)
    {
        if (strcmp(node->name, name) == 0)
        {
            LOG("Node %s found", name);
            return node;
        }
        node = node->sibling;
    }
    LOG("Node %s not found", name);
    return NULL;
}

int insert_child(Node* parent, Node* child)
{
    LOG("Inserting node %s into end of parent %s child list", child->name, parent->name);
    Node* node = parent->child;

    if (!node)
        parent->child = child;
    else
    {
        while (node->sibling)
            node = node->sibling;
        node->sibling = child;
    }
    child->parent  = parent;
    child->child   = NULL;
    child->sibling = NULL;
    return 0;
}

int split_path(char* path, char components_o[][16])
{
    LOG("Splitting path %s", path);
    char* s = strtok(path, "/");
    int n = 0;
    while (s)
    {
        strcpy(components_o[n++], s);
        s = strtok(NULL, "/");
    }
    LOG("Split path:");
    for (int i = 0; i < n; i++)
        LOG("%s", components_o[i]);
    return n;
}

Node* path2node(char* path)
{
    LOG("Finding node for path %s", path);
    char splits[64][16];
    int n = split_path(path, splits);

    Node* node;
    if (path[0] == '/')
    {
        LOG("Absolute path");
        node = root;
    }
    else
    {
        LOG("Relative path");
        node = cwd;
    }
    
    for (int i = 0; i < n; i++)
    {
        if (strcmp(splits[i], ".") == 0)
            continue;
        else if (strcmp(splits[i], "..") == 0)
            node = node->parent;
        else
            node = search_children(node, splits[i]);

        if (!node)
        {
            LOG("Node %s not found", path);
            return NULL;
        }
    }
    LOG("Node %s found", path);
    return node;
}

void node2path(Node* node, char* path_o)
{
    char components[64][16];
    int i = 0;
    while (node->parent)
    {
        strcpy(components[i++], node->name);
        node = node->parent;
    }

    path_o[0] = '/';
    path_o[1] = '\0';
    while (i--)
    {
        strcat(path_o, components[i]);
        if (i)
            strcat(path_o, "/");
    }
}

int create_item(char type, char* path)
{
    LOG("Creating %s %s", (type == 'D' ? "directory" : "file"), path);
    char temp[128];
    char dir[64];
    char name[64];

    strcpy(temp, path);
    strcpy(dir, dirname(temp));
    LOG("dirname: %s", dir);

    strcpy(temp, path);
    strcpy(name, basename(temp));
    LOG("basename: %s", name);

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
    {
        printf("Error: Path %s is not valid\n", path);
        return -1;
    }

    Node* parent = path2node(dir);
    if (parent->type != 'D')
    {
        printf("Error: Path %s is not valid\n", path);
        return -1;
    }

    if (search_children(parent, name))
    {
        printf("Error: %s %s already exists\n", (type == 'D' ? "directory" : "file"), path);
        return -1;
    }

    Node* node = make_node(name, type);
    insert_child(parent, node);
    return 0;
}

int remove_item(char type, char* path)
{
    LOG("Removing %s %s", (type == 'D' ? "directory" : "file"), path);
    Node* node = path2node(path);
    if (!node)
    {
        printf("Error: Could not find %s\n", path);
        return -1;
    }

    if (node->type != type)
    {
        printf("Error: %s is not a %s\n", path, (type == 'D' ? "directory" : "file"));
        return -1;
    }

    if (type == 'D' && node->child != NULL)
    {
        printf("Error: Unable to remove non-empty directory\n");
        return -1;
    }

    delete_node(node);
    return 0;
}

void write_tree(FILE* fp, Node* node)
{
    if (!node)
        return;

    char path[128];
    node2path(node, path);
    fprintf(fp, "%c %s\n", node->type, path);

    write_tree(fp, node->child);
    write_tree(fp, node->sibling);
}

/******************************************/
/**************** Commands ****************/
/******************************************/

int mkdir(char* path)
{
    return create_item('D', path);
}

int rmdir(char* path)
{
    return remove_item('D', path);
}

int cd(char* path)
{
    LOG("Changing working directory to %s", path);
    Node* node = path2node(path);

    if (!node)
    {
        printf("Error: Could not find %s\n", path);
        return -1;
    }

    if (node->type != 'D')
    {
        printf("Error: %s is not a directory\n", path);
        return -1;
    }

    LOG("Setting cwd to node %s ", node->name);
    cwd = node;
    return 0;
}

int ls(char* path)
{
    Node* node = path2node(path);
    if (!node)
    {
        printf("Error: Could not find %s\n", path);
        return -1;
    }

    if (node->type != 'D')
    {
        printf("Error: %s is not a directory", path);
        return -1;
    }

    if (node->child == NULL) // Directory is empty
        return 0;

    printf("type name\n");
    node = node->child;
    while (node)
    {
        printf("[%c]  %s\n", node->type, node->name);
        node = node->sibling;
    }
}

int pwd()
{
    char path[128];
    node2path(cwd, path);
    printf("%s\n", path);
    return 0;
}

int creat(char* path)
{
    return create_item('F', path);
}

int rm(char* path)
{
    return remove_item('F', path);
}

int save(char* fn)
{
    if (strlen(fn) == 0)
        fn = "fs.txt";
    LOG("Saving file system tree to %s", fn);

    FILE* oFile = fopen(fn, "w");
    if (!oFile)
    {
        printf("Error: Could not open %s\n", fn);
        return -1;
    }
    write_tree(oFile, root->child);

    fclose(oFile);
    return 0;
}

int reload(char* fn)
{
    if (strlen(fn) == 0)
        fn = "fs.txt";
    LOG("Loading file system tree from %s", fn);

    FILE* iFile = fopen(fn, "r");
    if (!iFile)
    {
        printf("Error: Could not open %s\n", fn);
        return -1;
    }
    char line[128];
    while (fgets(line, 128, iFile))
    {
        line[strlen(line) - 1] = '\0';
        char type;
        char path[128];
        sscanf(line, "%c %s", &type, path);

        LOG("Loading file system tree entry: [%c] %s", type, path);
        create_item(type, path);
    }

    fclose(iFile);
    return 0;
}

int menu()
{
    printf("[ ");
    for (int i = 0; CMD_NAMES[i]; i++)
    {
        printf("%s ", CMD_NAMES[i]);
        if (CMD_NAMES[i+1])
            printf("| ");
    }
    printf("]\n");
}

int tree()
{
    write_tree(stdout, root->child);
    return 0;
}

int reset()
{
    LOG("Resetting file system tree");
    while (root->child)
        delete_node(root->child);
}

int quit()
{
    LOG("Program exit");
    save("fs.txt");
    exit(0);
}
