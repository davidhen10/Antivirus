#include "main.h"
#include <string.h>

Database mkdatabase(int16 cap) {
    Database db;
    Entry *p;
    int16 size;

    db.num = 0;
    db.cap = cap;
    p = (Entry *)malloc($i size);
    assert(p);
    memset(p, 0, size);
    db.entries = p;
    db.entries = p;

    return db;
}

void destroydb(Database db) {
    db.cap = 0;
    db.num = 0;
    free(db.entries);

    return;
}

void showdb(Database db) {
    printf("cap:\t%d\nnum:\t%d\n",
        db.cap, db.num);
    
    return;
}

void addtodb(Database db, Entry e) {

}

int main(int argc, char *argv[]) {
    Database db;

    db = mkdatabase(50000);
    showdb(db);
    destroydb(db);
    
    return 0;
}