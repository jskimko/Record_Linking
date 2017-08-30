#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extract.h"

int main(int argc, char *argv[]) {
    entry_t *entries_1851, *entries_1881;

    // Check arguments
    if (argc != 3) {
        fprintf(stderr, "usage: %s data_1851 data_1881\n", __FILE__, __LINE__, argv[0]);
        return -1;
    }

    // Extract valid entries from data files
    entries_1851 = extract_valid_entries(argv[1], 1851);
    entries_1881 = extract_valid_entries(argv[2], 1881);

    if (!entries_1851 || !entries_1881) {
        fprintf(stderr, "%s:%d: could not extract valid entries\n", __FILE__, __LINE__);
        return -1;
    }

    return 0;
} // main

entry_t *extract_valid_entries(char *filename, int year) {
    FILE *fp;
    char buf[4096];
    char *col;
    int n_lines=0, count=0;
    int min_age, max_age;
    int i,j;
    entry_t *entries, *cur;

    // Open data file
    if ((fp = fopen(filename, "r")) == NULL) {
        perror("extract:fopen");
        return NULL;
    }

    // Remove header
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        perror("extract:fgets");
        return NULL;
    }

    // Count number of entries
    while (fgets(buf, sizeof(buf), fp)) n_lines++;

    // Go back to top of file and remove header
    fseek(fp, 0, SEEK_SET);
    fgets(buf, sizeof(buf), fp);

    // Set age limits
    if (year == 1851) {
        min_age = 0;
        max_age = 10;
    } else if (year == 1881) {
        min_age = 27;
        max_age = 43;
    } else {
        fprintf(stderr, "%s:%d: invalid year '%d' for data file\n", __FILE__, __LINE__, year);
    }

    // Store valid entries
    entries = malloc(sizeof(entry_t));
    cur = entries;
    for (i=0; i<n_lines; i++) { 
        unsigned int recID;
        char sex;
        unsigned char age;
        int len;

        fgets(buf, sizeof(buf), fp);

        // RecID = col 2
        col = strtok(buf, "\t");
        col = strtok(NULL, "\t");
        recID = atoi(col);

        // Sex = col 42
        for (j=0; j<39; j++) strtok(NULL, "\t");
        col = strtok(NULL, "\t");
        sex = col[0];
        if (sex == 'F') continue;

        // Age = col 44
        strtok(NULL, "\t");
        col = strtok(NULL, "\t"); 
        age = atoi(col);
        if (age < min_age || age > max_age) continue;

        // Std_Par = col 69
        for (j=0; j<24; j++) strtok(NULL, "\t");
        col = strtok(NULL, "\t");
        len = strlen(col);

        // Save valid entry
        entry_t *new_entry = malloc(sizeof(entry_t));
        new_entry->recID = recID;
        new_entry->sex = sex;
        new_entry->age = age;
        new_entry->par = malloc(len + 1);
        strncpy(new_entry->par, col, len + 1);

        cur->next = new_entry;
        cur = cur->next;
        count++;
    }

    cur = entries;
    while (cur->next) {
        cur = cur->next;
        print_entry(cur);
    }
    printf("There were %d extracted entries.\n", count);

    return entries;
} // extract_valid_entries

void print_entry(entry_t *entry) {
    printf("%d %c %d %s\n", entry->recID, entry->sex, entry->age, entry->par);
} // print_entry

