#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024

typedef struct FreeBlock {
    int index;
    struct FreeBlock *next, *prev;
} FreeBlock;

FreeBlock *head = NULL, *tail = NULL;
int freeCount = 0;

typedef struct Node {
    char name[51];
    int isDir, size, count;
    int blocks[1024];

    struct Node *next, *child, *parent;
} Node;

unsigned char disk[NUM_BLOCKS][BLOCK_SIZE];

Node *root, *cwd;

void addBlock(int index) {
    FreeBlock *n = malloc(sizeof(FreeBlock));
    n->index = index;
    n->next = NULL;
    n->prev = tail;

    if (!head) head = tail = n;
    else {
        tail->next = n;
        tail = n;
    }
    freeCount++;
}

int getBlock() {
    if (!head) return -1;

    FreeBlock *t = head;
    int index = t->index;
    head = head->next;

    if (head) head->prev = NULL;
    else tail = NULL;

    free(t);
    freeCount--;
    return index;
}

Node *newNode(char *name, int dir) {
    Node *n = calloc(1, sizeof(Node));
    strcpy(n->name, name);
    n->isDir = dir;
    return n;
}

Node *find(char *name) {
    if (!cwd->child) return NULL;

    Node *t = cwd->child;
    do {
        if (!strcmp(t->name, name)) return t;
        t = t->next;
    } while (t != cwd->child);

    return NULL;
}

void insert(Node *n) {
    n->parent = cwd;

    if (!cwd->child) {
        cwd->child = n;
        n->next = n;
    } else {
        Node *t = cwd->child;
        while (t->next != cwd->child) t = t->next;
        t->next = n;
        n->next = cwd->child;
    }
}

void removeNode(Node *n) {
    Node *t = cwd->child;

    if (t == n && t->next == t) {
        cwd->child = NULL;
    } else {
        while (t->next != n) t = t->next;
        t->next = n->next;
        if (cwd->child == n) cwd->child = n->next;
    }
}

void createEntry(char *name, int dir) {
    if (find(name)) {
        printf("Name already exists\n");
        return;
    }

    Node *n = newNode(name, dir);
    insert(n);

    printf("%s created successfully\n", name);
}

void writeFile(char *name, char *data) {
    Node *f = find(name);

    if (!f || f->isDir) {
        printf("File not found\n");
        return;
    }

    int size = strlen(data);
    int needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (needed > freeCount + f->count) {
        printf("Disk full\n");
        return;
    }

    for (int i = 0; i < f->count; i++)
        addBlock(f->blocks[i]);

    f->count = 0;

    for (int i = 0; i < needed; i++)
        f->blocks[i] = getBlock();

    f->count = needed;
    f->size = size;

    for (int i = 0; i < size; i++)
        disk[f->blocks[i / BLOCK_SIZE]][i % BLOCK_SIZE] = data[i];

    printf("Data written successfully\n");
}

void readFile(char *name) {
    Node *f = find(name);

    if (!f || f->isDir) {
        printf("File not found\n");
        return;
    }

    for (int i = 0; i < f->size; i++)
        putchar(disk[f->blocks[i / BLOCK_SIZE]][i % BLOCK_SIZE]);

    printf("\n");
}

void deleteEntry(char *name, int dir) {
    Node *n = find(name);

    if (!n || n->isDir != dir) {
        printf("Entry not found\n");
        return;
    }

    if (dir && n->child) {
        printf("Directory not empty\n");
        return;
    }

    if (!dir) {
        for (int i = 0; i < n->count; i++)
            addBlock(n->blocks[i]);
    }

    removeNode(n);
    free(n);
    printf("Deleted successfully\n");
}

void ls() {
    if (!cwd->child) {
        printf("(empty)\n");
        return;
    }

    Node *t = cwd->child;
    do {
        printf("%s %s\n", t->isDir ? "[DIR]" : "[FILE]", t->name);
        t = t->next;
    } while (t != cwd->child);
}

void pwd() {
    Node *a[100];
    int i = 0;

    Node *t = cwd;
    while (t != root && i < 100) {
        a[i++] = t;
        t = t->parent;
    }

    printf("/");
    while (i > 0)
        printf("%s%s", a[--i]->name, i ? "/" : "");
    printf("\n");
}

void df() {
    int used = NUM_BLOCKS - freeCount;
    printf("Total: %d\nUsed: %d\nFree: %d\n",
           NUM_BLOCKS, used, freeCount);
    printf("Usage: %.2f%%\n", used * 100.0 / NUM_BLOCKS);
}

void cleanup(Node *dir) {
    if (!dir->child) return;

    Node *start = dir->child;
    Node *t = start;

    do {
        Node *next = t->next;
        cleanup(t);
        free(t);
        t = next;
    } while (t != start);
}

int main() {
    char cmd[1024], name[51], data[1024];

    for (int i = 0; i < NUM_BLOCKS; i++)
        addBlock(i);

    root = newNode("/", 1);
    cwd = root;

    printf("VFS Ready\n");

    while (1) {
        pwd();
        printf("> ");

        if (!fgets(cmd, sizeof(cmd), stdin)) break;
        cmd[strcspn(cmd, "\n")] = 0;

        if (!strcmp(cmd, "exit")) break;
        else if (!strcmp(cmd, "ls")) ls();
        else if (!strcmp(cmd, "pwd")) pwd();
        else if (!strcmp(cmd, "df")) df();

        else if (sscanf(cmd, "mkdir %50s", name) == 1)
            createEntry(name, 1);

        else if (sscanf(cmd, "create %50s", name) == 1)
            createEntry(name, 0);

        else if (sscanf(cmd, "cd %50s", name) == 1) {
            if (!strcmp(name, ".."))
                cwd = cwd->parent ? cwd->parent : root;
            else if (!strcmp(name, "/"))
                cwd = root;
            else {
                Node *n = find(name);
                if (n && n->isDir) cwd = n;
                else printf("Directory not found\n");
            }
        }

        else if (sscanf(cmd, "delete %50s", name) == 1)
            deleteEntry(name, 0);

        else if (sscanf(cmd, "rmdir %50s", name) == 1)
            deleteEntry(name, 1);

        else if (sscanf(cmd, "read %50s", name) == 1)
            readFile(name);

        else if (!strncmp(cmd, "write ", 6)) {
            char *p = strchr(cmd + 6, '"');
            if (sscanf(cmd + 6, "%50s", name) == 1 && p) {
                char *end = strrchr(p + 1, '"');
                if (end) {
                    *end = '\0';
                    writeFile(name, p + 1);
                } else printf("Missing closing quote\n");
            } else printf("Usage: write file \"data\"\n");
        }

        else printf("Invalid command\n");
    }

    cleanup(root);
    free(root);

    while (head) {
        FreeBlock *t = head;
        head = head->next;
        free(t);
    }

    printf("Memory released. Exiting...\n");
    return 0;
}