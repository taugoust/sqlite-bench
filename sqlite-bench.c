#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define NS_PER_S 1000000000u
#define MS_PER_S 1000000u

// #define DEBUG
// #define PERF
// #define WAL

#define A_TEST
#define B_TEST

#ifdef PERF
static inline void __perf_start()
{
	asm volatile("int $0xfd");
}

static inline void __perf_stop()
{
	asm volatile("int $0xfe");
}
#endif

typedef unsigned long int ticks_t;

struct filemap
{
    void *ptr;
    size_t size;
};

struct lr
{
    struct filemap *fm;
    const char *cur;
    char buffer[512];
};

static inline ticks_t ticks()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ticks_t)((ts.tv_sec * NS_PER_S) + ts.tv_nsec);
}

static int exec(sqlite3 *db, const char *sql)
{
    return sqlite3_exec(db, sql, 0, 0, 0) == SQLITE_OK;
}

static struct filemap *mapfile(const char *file)
{
    int fd = open(file, O_RDONLY);
    if (fd < 0)
    {
        return NULL;
    }

    struct filemap *fm = malloc(sizeof(struct filemap));
    if (!fm)
    {
        close(fd);
        return NULL;
    }

    struct stat st;
    fstat(fd, &st);

    fm->size = st.st_size;
    fm->ptr = mmap(NULL, fm->size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (fm->ptr == MAP_FAILED)
    {
        free(fm);
        return NULL;
    }

    return fm;
}

static void unmapfile(struct filemap *fm)
{
    munmap(fm->ptr, fm->size);
    free(fm);
}

static struct lr *newlr(struct filemap *fm)
{
    struct lr *lr = malloc(sizeof(struct lr));
    lr->fm = fm;
    lr->cur = (const char *)fm->ptr;

    return lr;
}

static void dellr(struct lr *lr)
{
    free(lr);
}

static const char *lrread(struct lr *lr)
{
    if (lr->cur >= (const char *)lr->fm->ptr + lr->fm->size)
    {
        return NULL;
    }

    int n = 0;
    while (*lr->cur != '\n')
    {
        lr->buffer[n++] = *lr->cur;
        lr->cur++;
    }

    lr->cur++;
    lr->buffer[n] = 0;

    return lr->buffer;
}

// ********************************************
// Test A: Batch Exec (1 exec for all requests)
// ********************************************
static int run_test_a(int idx, sqlite3 *db, const char *file)
{
    struct filemap *sqlfm = mapfile(file);
    if (!sqlfm)
    {
        return 1;
    }

    ticks_t start = ticks();

#ifdef PERF
    __perf_start();
#endif
    exec(db, (const char *)sqlfm->ptr);
#ifdef PERF
    __perf_stop();
#endif

    ticks_t end = ticks();

#ifdef DEBUG
    printf("  * %d-a time: %lu ms\n", idx, (end - start) / MS_PER_S);
#else
#ifndef PERF
    printf("%d-batch;%lu\n", idx, (end - start) / MS_PER_S);
#endif
#endif

    unmapfile(sqlfm);
    return SQLITE_OK;
}

// ********************************************
// Test B: Multi Exec (1 exec per request)
// ********************************************
static ticks_t testb_ticks;
static int run_test_b(int idx, sqlite3 *db, const char *file)
{
    struct filemap *sqlfm = mapfile(file);
    if (!sqlfm)
    {
        return 1;
    }

    struct lr *lr = newlr(sqlfm);
    const char *line;

    ticks_t loop_start = ticks();
    /* unsigned long count = 0; */
    while ((line = lrread(lr)))
    {
        /* ticks_t stmt_start = ticks(); */
        exec(db, line);
        /* ticks_t stmt_end = ticks(); */
        /* stmt_total += stmt_end - stmt_start; */
        /* count++; */
    }
    ticks_t loop_end = ticks();

#ifdef DEBUG
    printf("  loop-total: %lu ms\n", (loop_end - loop_start) / MS_PER_S);
    printf("  * %d.2 stmt-total: %lu ms\n", idx, stmt_total / MS_PER_S);
    printf("  stmt-per: %lu ms\n", stmt_total / count / MS_PER_S);
#else
    ticks_t ticks_count = (loop_end - loop_start) / MS_PER_S;
    printf("%d-multi;%lu\n", idx, ticks_count);
    testb_ticks += ticks_count;
#endif

    dellr(lr);

    unmapfile(sqlfm);
    return SQLITE_OK;
}

int main(int argc, char **argv)
{
    sqlite3 *db;
    int rc;

    if (argc < 2)
    {
        fprintf(stderr, "error: usage: %s <sql-files>\n", argv[0]);
        return 1;
    }

    const char *dbfile = "test.db";
    unlink(dbfile);

    // ---------------- A TEST ---------------- //

/*     rc = sqlite3_open(dbfile, &db); */
/*     if (rc != SQLITE_OK) */
/*     { */
/*         fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db)); */
/*         sqlite3_close(db); */

/*         return 1; */
/*     } */

/* #ifdef WAL */
/*     exec(db, "PRAGMA journal_mode=WAL;"); */
/* #endif */

/* #ifdef A_TEST */
/*     for (int i = 1; i < argc; i++) */
/*     { */
/* #ifdef DEBUG */
/*         printf("----------------\ntest %d-a:\n", i); */
/*         ticks_t test_start = ticks(); */
/* #endif */
/*         rc = run_test_a(i, db, argv[i]); */
/* #ifdef DEBUG */
/*         ticks_t test_end = ticks(); */
/*         printf("test %d-a duration: %lu ms\n", i, (test_end - test_start) / MS_PER_S); */
/* #endif */
/*     } */
/* #endif */

/*     sqlite3_close(db); */
/*     unlink(dbfile); */

    // ---------------- B TEST ---------------- //

    rc = sqlite3_open(dbfile, &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }

#ifdef WAL
    exec(db, "PRAGMA journal_mode=WAL;");
#endif

#ifdef B_TEST
    for (int i = 1; i < argc; i++)
    {
#ifdef DEBUG
        printf("----------------\ntest %d-b:\n", i);
        ticks_t test_start = ticks();
#endif
        rc = run_test_b(i, db, argv[i]);
#ifdef DEBUG
        ticks_t test_end = ticks();
        printf("test %d-b duration: %lu ms\n", i, (test_end - test_start) / MS_PER_S);
#endif
    }
    printf("total-multi;%lu\n", testb_ticks);
#endif

    sqlite3_close(db);
    unlink(dbfile);

    return rc;
}
