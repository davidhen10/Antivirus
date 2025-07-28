#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/syscall.h>
#include <time.h>

typedef unsigned char int8;
typedef unsigned short int int16;
typedef unsigned int int32;
typedef unsigned long long int int64;

#define Blocksize 50000

#define $1 (int8 *)
#define $2 (int16)
#define $4 (int32)
#define $8 (int64)
#define $c (char *)
#define $i (int)

#define onedot(x) ((*(x) == '.') && !(*(x + 1)))
#define twodots(x) ((*(x) == '.') && (*(x + 1) == '.') && !(*(x + 2)))
#define log(f, args...) printf(f, args); fflush(stdout)

typedef int8 Dir[64];
typedef int8 File[32];
typedef unsigned long long int Timestamp;

enum e_filetype {
    file  = 1,
    dir   = 2,
    other = 3
}
typedef enum e_filetype Filetype;

struct s_entry {
    Filetype type;
    Dir dir;
    File file;
    Timestamp lastscanned;
};
typedef struct s_entry Entry;

struct s_database {
    Entry *entries;
    int32 cap;
    int32 num;
};

typedef struct s_database Database;
typedef bool (*function)(Entry);

#define linux_dirent dirent

void prepare(void);
Timestamp unixtime(void);
Database *filter(Database*, function);
Database *mkdatabase(void);
bool iself(Entry);
bool adddir(Database*, int8*);
void addtodb(Database*, Entry);
void destroydb(Database*);
void showdb(Database*);
int main(int, char**);