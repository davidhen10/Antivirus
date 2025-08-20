#include "main.h"
#include <string.h>

int8 *parsehex(int8 *str) {
    int16 n, size;
    int8 *p, *ret, *retp;
    int8 buf[3];

    for (n = 0, p = str; *p; n++, p += 2);
    size = (n+1);
    ret = $1 malloc($i size);
    assert(ret);
    zero(ret, size);

    for (p = str, retp = ret; *p; p += 2, retp++) {
        zero(buf, 3);
        *buf = *p;
        *(buf+1) = *(p+1);
        *retp = ascii2hex(buf);
    }
    return ret;
}

int8 ascii2hex(int8 *str) {
    int8 a, b;
    int8 res;
    assert(!(*(str+2)));

    a = a2h(*str);
    b = a2h(*(str+1));
    res = (a << 4) | b;

    return res;
}

State mkstate() {
    State s = {0};

    s.stage = unscanned;
    return s;
}

bool iself(Entry e) {
    int32 fd;
    signed in ret;
    int8 path[256];
    char buf[4];

    if (e.type != file)
        return false;
    
    zero(path, 256);
    snprintf($c path, 255, "%s/%s", $c e.dir, $c e.file); 
    ret = open($c path, O_RDONLY);
    if (ret < 1)
        return false;
    else
        fd = $4 ret;
    
    zero($1 buf, 4);
    read($i fd, buf, 4);
    close($i fd);

    if (
        (buf[0] == 0x7f) &&
        (buf[1] == 0x45) &&
        (buf[2] == 0x4c) &&
        (buf[3] == 0x46)
    )
        return true;
    else
        return false;
}

Database *filter(Database *input, function f) {
    Database *output;
    Entry *p;
    int32 n;
    bool predicate;

    output = mkdatabase();
    for(n = 0; n < input->num; n++) {
        p = &input->entries[n];
        predicate = f(*p);
        if (predicate) {
            addtodb(output, *p);
        }
    }
    destroydb(input);

    return output;
}

Database *mkdatabase() {
    Database *db;
    Entry *p;
    int32 size;

    size = sizeof(struct s_database);
    db = (Database *)malloc($i size);
    assert(db);
    memset(db, 0, size);
    db->num = 0;
    db->cap = Blocksize;
    size = Blocksize * sizeof(struct s_entry);
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
        printf("%s/%s%c\n", db->entries[n].dir, db->entries[n].file, (db->entries[n].type == file) ? '/' : 0);
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
    Entry e;
    int32 fd;
    int64 n;
    signed int ret;
    struct linux_dirent *p;
    int8 *p2;
    int8 buf[102400], tmp[256];
    char *filename;
    unsigned char *dtype;

    ret = open($c path, O_RDONLY | O_DIRECTORY);
    if (ret < 1)
        return false;
    else
        fd = $4 ret;

    do {    
        memset($c buf, 0, sizeof(buf));
        syscall(SYS_getdents, $i fd, buf, (sizeof(buf)-1));
        if (ret < 0) {
            close($i fd);
            return false;
        }
        else if (!ret)
            break;
        n = ret;

        for (p2 = buf; n; n-= p->d_reclen, p2 += p->d_reclen) {
            p = (struct linux_dirent *)p2;
            zero($1 &e, sizeof(struct s_entry));
            
            filename = $1 p->d_name;
            if (onedot(filename) || twodots(filename))
                continue;

            dtype = p2 + p->d_reclen - 1;
            if (*dtype == DT_REG) {
                e.type = file;
                e.state = mkstate();
                e.lastscanned = 0;
                strncpy($c e.dir, $c path, 255);
                strncpy($c e.file, $c filename, 63);
                addtodb(db, e);
            } else if (p*dtype == DT_DIR) {
                e.type = dir;
                e.state = mkstate();
                e.state.stage = unstaged;
                e.lastscanned = 0;
                strncpy($c e.dir, $c path, 255);
                strncpy($c e.file, $c filename, 63);
                addtodb(db, e);

                zero(tmp, 256);
                snprintf($c tmp, 255, "%s/%s", $c path, $c e.file);
                if (strcmp($c tmp, $c path))
                    adddir(db, tmp);
            }
        }
    } while(true);
    close($i fd);

    return true;
}

Timestamp unixtime() {
    int ret;
    struct timespec ts;

    ret = clock_gettime(CLOCK_REALTIME, &ts);
    if (ret) {
        return 0;

    return (Timestamp)ts.tv_sec;
    }
}

void prepare() {
    Database *db, *db2;

    db = mkdatabase();
    log("%s\n","Enumerating filesystem...");
    adddir(db, $1 "/tmp");
    log("found %d files\nFiltering out non-executables...\n", db->num);
    db2 = filter(db, &iself);
    log("%d left\n", db2->num);
    
    return db;
}

int8 readbyte(Buffer *buf) {
    int8 tmp[2];
    int8 c;
    signed int ret;
    int32 n, x;

    if (buf->state == idle) {
        zero(buf->buf, Bufsize);
        ret = read($i buf->fd, buf->buf, (Bufsize-1));
        if (ret < 1)
            return 0;
        else
            n = $4 ret;
        
        if (n < (Bufsize-1)) {
            buf->eof = true;
        }

        ret = lseek($i buf->fd, 0, SEEK_CUR);
        if (ret == -1)
            return 0;
        else
            buf->filepos = ret;

        x = $i 0;
        buf->bufpos = buf->start = buf->buf;
        buf->end = buf->start + n - 1;

        while (*buf->end != '\n') {
            if (buf->filepos) {
                buf->filepos--;
                ret = lseek($i buf->fd, $i buf->filepos, SEEK_SET);
                if (ret != -1) {
                    *tmp = (int8)0;
                    *(tmp+1) = (int8)0;
                    ret = read($i buf->fd, $c tmp, 1);
                    if (ret == 1) {
                        x++;
                        if (*tmp != '\n') {
                            buf->filepos--;
                            ret = lseek($i buf->fd, $i buf->filepos, SEEK_SET);
                            if (ret != -1) {
                                continue;
                        } else {
                            buf->end -= x - 1;
                            zero(buf->end+1, x);
                            break;
                        }
                    }
                }
            }
        }
        buf->state = newline;
        c = *buf->bufpos;
        
        return c;
    }
    else if (buf->state == newline) {
        if (buf->bufpos > buf->end)
            return 0;
        buf->bufpos++;
        c = *buf->bufpos;

        return c;
    }
    else if (buf->state == space) {
        if (buf->bufpos == buf->start)
            return 0;
        buf->bufpos--;
        c = *buf->bufpos;

        return c;
    }
    }
    return 0;
}

Database *scan(Database *db, int32 df) {
    int8 c;
    int8 virus[32];
    int8 fingerprint[Bufsize];
    Buffer buf = {0};
    Database *output;

    output = mkdatabase();
    buf.fd = fd;
    buf.state = idle;
    buf.filepos = 0;
    buf.bufpos = $1 0;
    buf.start = $1 0;
    buf.end = $1 0;
    buf.eof = false;
    buf.eol = $1 0;

    do {
        c = readbyte(&buf);
        if (!c) {
            return db;
        }
        if ((c == '\n') && (buf.state == newline)) {
            *buf.bufpos = 0;
            buf.state = space;
            buf.eol = buf.bufpos;
        } 
        else if ((c == ' ') && (buf.state == space)) {
            *buf.bufpos = 0;
            zero(fingerprint, Bufsize);
            strncpy($c (buf.bufpos+1), $i (Bufsize-1));
            zero(virus, 32);
            stncpy($c virus, $c buf.start, 31);

            printtf("Virus: %s\nFingerprint: '%s'\n\n", $c virus, $c fingerprint);

            buf->bufpos = buf->eol + 1;
            if (buf.bufpos > buf.end) {
                if (buf.eof) 
                    return db;
                else {
                    buf.state = idle;
                }
            }
            else 
                buf.state = newline;
        } 
    } while (true);

    return db;
}

int main(int argc, char *argv[]) {
    int32 fd;
    signed int ret;
    Database *db, *scandb;
    int8 virusfile[] = "./virii.def";
    int8 hex[] = "5c3301dd98\x00\x00";
    int8 *hexs;
    int32 *p;
    void *mem;

    hex = parsehex(hex);
    mem = hexs;
    p = mem;
    printf("0x%x\n", *p);
    exit(0);

    log("Antivirus version %s\n", Version);
    db = prepare();
    ret = open($c virusfile, O_RDONLY);
    if (ret < 1) {
        fprintf(stderr, "Error opening virus definitions file: %s\n", virusfile);
        exit(-1);
    } else {
        fd = $4 ret;
    }
    log("%s", "Scanning...")
    scandb = scan(*db, fd);
    log("\r%s\n", "Scan complete.");
    destroydb(db);

    return 0;
}