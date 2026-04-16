#include "index.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// ─── PROVIDED ────────────────────────────────────────────────────────────────

IndexEntry* index_find(Index *index, const char *path) {
    for (int i = 0; i < index->count; i++) {
        if (strcmp(index->entries[i].path, path) == 0)
            return &index->entries[i];
    }
    return NULL;
}

int index_remove(Index *index, const char *path) {
    for (int i = 0; i < index->count; i++) {
        if (strcmp(index->entries[i].path, path) == 0) {
            int remaining = index->count - i - 1;
            if (remaining > 0)
                memmove(&index->entries[i], &index->entries[i + 1],
                        remaining * sizeof(IndexEntry));
            index->count--;
            return index_save(index);
        }
    }
    return -1;
}

// ─── IMPLEMENTATION ─────────────────────────────────────────────────────────

// LOAD INDEX
int index_load(Index *index) {
    FILE *f = fopen(".pes/index", "r");
    if (!f) {
        index->count = 0;
        return 0;
    }

    index->count = 0;

    while (index->count < MAX_INDEX_ENTRIES) {
        IndexEntry *e = &index->entries[index->count];

        char hash_hex[HASH_HEX_SIZE + 1];

        int ret = fscanf(f, "%o %64s %ld %ld %255s\n",
                         &e->mode,
                         hash_hex,
                         &e->mtime_sec,
                         &e->size,
                         e->path);

        if (ret != 5) break;

        hex_to_hash(hash_hex, &e->hash);

        index->count++;
    }

    fclose(f);
    return 0;
}

// SORT HELPER
static int compare_index_entries(const void *a, const void *b) {
    return strcmp(((const IndexEntry *)a)->path,
                  ((const IndexEntry *)b)->path);
}

// SAVE INDEX
int index_save(const Index *index) {
    char tmp_path[] = ".pes/index.tmp";

    FILE *f = fopen(tmp_path, "w");
    if (!f) return -1;

    Index sorted = *index;
    qsort(sorted.entries, sorted.count, sizeof(IndexEntry), compare_index_entries);

    for (int i = 0; i < sorted.count; i++) {
        const IndexEntry *e = &sorted.entries[i];

        char hex[HASH_HEX_SIZE + 1];
        hash_to_hex(&e->hash, hex);

        fprintf(f, "%o %s %ld %ld %s\n",
                e->mode,
                hex,
                e->mtime_sec,
                e->size,
                e->path);
    }

    fflush(f);
    fsync(fileno(f));
    fclose(f);

    rename(tmp_path, ".pes/index");

    return 0;
}

// ADD FILE TO INDEX

}
