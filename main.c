#include "main.h"
#include <string.h>

Database *mkdatabase(int32 cap) {
    Database *db;
    Entry *p;
    int32 size;

    size = sizeof(struct s_database);
    db = (Database *)malloc($i size);
    assert(db);
    memset(db, 0, size);
    db->num = 0;
    db->cap = cap;
    size = cap * sizeof(struct s_entry);
    p = (Entry *)malloc(size);
    assert(p);
    memset(p, 0, size);
    db->entries = p;

    return db;
}

void destroydb(Database *db) {
    db->cap = 0;
    db->num = 0;
    free(db->entries);
    free(db);

    return;
}

void showdb(Database *db) {
    int32 n;
    printf("cap:\t%d\nnum:\t%d\n",
        db->cap, db->num);

    for(n = 0; n < db->num; n++) {
        printf("%s/%s\n", db->entries[n].dir, db->entries[n].file);
    }
    return;
}

void addtodb(Database *db, Entry e) {
    int32 size, cap, ix;

    if (db->num == db->cap) {
        cap = db->cap + Blocksize;
        size = cap * sizeof(struct s_entry);
        db->entries = realloc(db->entries, size);
        assert(db->entries);
        db->cap = cap;
    }

    ix = db->num;
    memcpy($c &db->entries[ix], $c &e, sizeof(struct s_entry));
    db->num++;

    return;
}

bool adddir(Database *db, int8 *path) {
    int32 fd;
    signed int ret;
    struct linux_dirent *p;
    int8 buf[102400];

    ret = open($c path, O_RDONLY | O_DIRECTORY);
    if (ret < 1)
        return false;
    else
        fd = $4 ret;

    memset(buf, 0, sizeof(buf));
    syscall(SYS_getdents, $i fd, buf, (sizeof(buf)-1));
    if (ret < 0) {
        close($i fd);
        return false;
    }
}

int main(int argc, char *argv[]) {
    Database *db; 
    Entry e1, e2;

    strncpy(e1.dir, "/tmp", 63);
    strncpy(e2.dir, "/tmp", 63);
    strncpy(e1.file, "FILE1.txt", 31);
    strncpy(e2.file, "anotherfile.txt", 31);

    db = mkdatabase(50000);
    addtodb(db,e1);
    addtodb(db,e2);
    showdb(db);
    destroydb(db);
    
    return 0;
}