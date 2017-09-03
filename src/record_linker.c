#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_linker.h"
#include "jarowinkler.h"

int main(int argc, char *argv[]) {
    entry_t *entries_1851, *entries_1881;
    match_t *matches;

    // Check arguments
    if (argc != 6) {
        fprintf(stderr, "usage: %s data_1851 data_1881 names_1851 names_1881 "
                        "fnames_std\n", __FILE__, __LINE__, argv[0]);
        return -1;
    }

    // Extract valid entries from data files
    entries_1851 = extract_valid_entries(argv[1], 1851);
    entries_1881 = extract_valid_entries(argv[2], 1881);

    if (!entries_1851 || !entries_1881) {
        fprintf(stderr, "%s:%d: could not extract valid entries\n", __FILE__, __LINE__);
        return -1;
    }

    // Add names
    add_names(argv[3], entries_1851);
    add_names(argv[4], entries_1881);

    // Standardize fnames
    standardize_fnames(argv[5], entries_1851);
    standardize_fnames(argv[5], entries_1881);
    return 0;

#ifdef PRINT
    // Print valid entries from each list
    printf("Printing valid entries:\n");
    print_entries(entries_1851);
    print_entries(entries_1881);
#endif

    // Find matches
    matches = find_matches(entries_1851, entries_1881);

#ifdef PRINT
    // Print matches
    printf("Printing matches:\n");
    print_matches(matches);
#endif

    return 0;
} // main

entry_t *extract_valid_entries(char *filename, int year) {
    FILE *fp;
    char buf[4096];
    char *col;
    int n_lines=0;
    int min_age, max_age;
    int i,j;
    entry_t *entries, *cur;

    unsigned int recID;
    char sex;
    unsigned char age;
    int len;

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
    cur = entries = malloc(sizeof(entry_t));
    cur->next = NULL;

    for (i=0; i<n_lines; i++) { 
        fgets(buf, sizeof(buf), fp);

        // RecID = col 2
        strtok(buf, "\t");
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
        new_entry->par = malloc(len+1);
        strncpy(new_entry->par, col, len+1);
        new_entry->fname = NULL;
        new_entry->lname = NULL;
        new_entry->next = NULL;

        cur->next = new_entry;
        cur = cur->next;
    }

    fclose(fp);
    return entries;
} // extract_valid_entries

void add_names(char *filename, entry_t *entries) {
    FILE *fp;
    char buf[512];
    char *col;
    int len;
    int recID;

    // Open data file
    if ((fp = fopen(filename, "r")) == NULL) {
        perror("extract:fopen");
        return;
    }

    // Remove header
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        perror("extract:fgets");
        return;
    }

    // Go to first entry
    entries = entries->next;

    // Scan for matching RecID
    while (entries) {
        if (!fgets(buf, sizeof(buf), fp)) break;

        // RecID = col 2
        strtok(buf, "\t");
        col = strtok(NULL, "\t");
        recID = atoi(col);

        if (entries->recID == recID) {
            // Pname = col 4
            strtok(NULL, "\t");
            col = strtok(NULL, "\t");

            len = strlen(col);
            entries->fname = malloc(len+1);
            strncpy(entries->fname, col, len+1);

            // Sname = col 6
            strtok(NULL, "\t");
            col = strtok(NULL, "\t");
            len = strlen(col);

            entries->lname = malloc(len+1);
            strncpy(entries->lname, col, len+1);
            
            entries = entries->next;
        }
    }

    fclose(fp);
} // add_names

void standardize_fnames(char *filename, entry_t *entries) {
    FILE *fp;
    char buf[512];
    char *col, c;
    name_dict_t *name_dict, *cur;
    int len, i;

    // Open data file
    if ((fp = fopen(filename, "r")) == NULL) {
        perror("extract:fopen");
        return;
    }

    // Remove header
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        perror("extract:fgets");
        return;
    }

    // Create name dictionary to map fname -> fname_std
    cur = name_dict = malloc(sizeof(name_dict_t));
    cur->next = NULL;

    while (fgets(buf, sizeof(buf), fp)) {
        // create new mapping
        name_dict_t *new_map = malloc(sizeof(name_dict_t));
        new_map->next = NULL;

        // fname
        col = strtok(buf, ",");
        len = strlen(col);
        new_map->fname = malloc(len+1);
        strncpy(new_map->fname, col, len+1);

        // fname_std
        col = strtok(NULL, "\n");
        len = strlen(col);
        new_map->fname_std = malloc(len+1);
        strncpy(new_map->fname_std, col, len+1);

        cur->next = new_map;
        cur = cur->next;
    }

    printf("Printing standardized name dictionary:\n");
    cur = name_dict;
    while (cur->next) {
        cur = cur->next;
        printf("%s %s\n", cur->fname, cur->fname_std);
    }

    fclose(fp);
}

match_t *find_matches(entry_t *entries_1851, entry_t *entries_1881) {
    match_t *ret, *cur_ret;
    entry_t *cur_1851, *cur_1881, *new_entry;
    
    cur_ret = ret = malloc(sizeof(match_t));
    cur_1851 = entries_1851;
    cur_1881 = entries_1881;

    // for each 1851 entry
    while (cur_1851->next) {
        cur_1851 = cur_1851->next;

        // check each 1881 entry for satisfiability
        while (cur_1881->next) {
            cur_1881 = cur_1881->next;

            // age criteria
            if (cur_1851->age + 30 - cur_1881->age > 5) continue;

            // jarowinkler criteria
            if (!cur_1851->fname || !cur_1881->fname) continue;
            if (1-jarowinkler(cur_1851->fname, cur_1881->fname) > 0.2) continue;
            if (!cur_1851->lname || !cur_1881->lname) continue;
            if (1-jarowinkler(cur_1851->lname, cur_1881->lname) > 0.2) continue;
            
            // save match
            match_t *new_match = malloc(sizeof(match_t));
            new_match->entry_1851 = cur_1851;
            new_match->entry_1881 = cur_1881;
            new_match->next = NULL;

            cur_ret->next = new_match;
            cur_ret = cur_ret->next;

        }

        // reset 1881 pointer
        cur_1881 = entries_1881;
    }

    return ret;
} // find_matches

#ifdef PRINT
void print_entries(entry_t *entries) {
    int count=0;

    while (entries->next) {
        entries = entries->next;
        printf("%d %s %s %c %d %s\n", entries->recID, entries->fname, entries->lname,
                                      entries->sex, entries->age, entries->par);
        count++;
    }
    printf("There were %d extracted entries.\n\n", count);
} // print_entries

void print_matches(match_t *matches) {
    int count=0;

    while (matches->next) {
        matches = matches->next;
        print_entries(matches->entry_1851);
        print_entries(matches->entry_1881);
        count++;
    }
    printf("There were %d extracted matches.\n", count);
} // print_matches

#endif
