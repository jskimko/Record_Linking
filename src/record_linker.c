#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "record_linker.h"
#include "jarowinkler.h"
#include "name_dict.h"

int main(int argc, char *argv[]) {
    entry_t *entries_1851, *entries_1881;
    name_dict_t *name_dict;
    match_t *matches;
    int rc;

    clock_t start;

    // Check arguments
    if (argc != 6) {
        fprintf(stderr, "usage: %s data_1851 data_1881 names_1851 names_1881 "
                        "fnames_std\n", argv[0]);
        return -1;
    }

    // Extract valid entries from data files
    fprintf(stderr, "Extracting file 1 ...... "); start = clock();
    entries_1851 = extract_valid_entries(argv[1], 1851);
    fprintf(stderr, "%9lfs\n", (double) (clock() - start) / CLOCKS_PER_SEC); 
    fprintf(stderr, "Extracting file 2 ...... "); start = clock();
    entries_1881 = extract_valid_entries(argv[2], 1881);
    fprintf(stderr, "%9lfs\n", (double) (clock() - start) / CLOCKS_PER_SEC);

    if (!entries_1851 || !entries_1881) EXIT_WITH_ERROR("could not extract valid entries");

    // Add names to valid entries
    fprintf(stderr, "Adding names 1 ......... "); start = clock();
    rc = add_names(argv[3], entries_1851);
    fprintf(stderr, "%9lfs\n", (double) (clock() - start) / CLOCKS_PER_SEC);
    if (rc == -1) EXIT_WITH_ERROR("could not add names to entries");

    fprintf(stderr, "Adding names 2 ......... "); start = clock();
    rc = add_names(argv[4], entries_1881);
    fprintf(stderr, "%9lfs\n", (double) (clock() - start) / CLOCKS_PER_SEC);
    if (rc == -1) EXIT_WITH_ERROR("could not add names to entries");

    // Generate name dictionary
    name_dict = generate_name_dict(argv[5]);
    if (name_dict == NULL) EXIT_WITH_ERROR("could not generate name dictionary");

#ifdef PRINT
    printf("Printing standardized name dictionary:\n");
    print_name_dict(name_dict);
#endif

    // Standardize fnames
    fprintf(stderr, "Standardizing names 1 .. "); start = clock();
    standardize_fnames(entries_1851, name_dict);
    fprintf(stderr, "%9lfs\n", (double) (clock() - start) / CLOCKS_PER_SEC);

    fprintf(stderr, "Standardizing names 2 .. "); start = clock();
    standardize_fnames(entries_1881, name_dict);
    fprintf(stderr, "%9lfs\n", (double) (clock() - start) / CLOCKS_PER_SEC);

    // Free name dictionary
    free_name_dict(name_dict);

#ifdef PRINT
    // Print valid entries from each list
    printf("Printing valid entries from file 1:\n");
    print_entries(entries_1851);
    printf("Printing valid entries from file 2:\n");
    print_entries(entries_1881);
#endif

    // Find matches
    fprintf(stderr, "Finding matches ........ "); start = clock();
    matches = find_matches(entries_1851, entries_1881);
    fprintf(stderr, "%9lfs\n", (double) (clock() - start) / CLOCKS_PER_SEC);

    // Print matches
    printf("Printing matches:\n");
    print_matches(matches);

    // Free data
    free_entries(entries_1851);
    free_entries(entries_1881);
    free_matches(matches);

    return 0;
} // main

/* Extracts entries from data file where valid means male and below age 10. */
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
        return NULL;
    }

    // Store valid entries
    cur = entries = malloc(sizeof(entry_t));
    cur->fname = cur->lname = NULL; cur->par = NULL;
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
        strcpy(new_entry->par, col);
        new_entry->fname = NULL;
        new_entry->lname = NULL;
        new_entry->next = NULL;

        cur->next = new_entry;
        cur = cur->next;
    }

    fclose(fp);
    return entries;
} // extract_valid_entries

/* Reads the name file and adds names to a list of valid entries. */
int add_names(char *filename, entry_t *entries) {
    FILE *fp;
    char buf[512];
    char *col;
    int len;
    int recID;
    char name[32]; // stores stripped names

    // Open data file
    if ((fp = fopen(filename, "r")) == NULL) {
        perror("extract:fopen");
        return -1;
    }

    // Remove header
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        perror("extract:fgets");
        return -1;
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

        while (entries->recID < recID) {
            entries = entries->next;
            if (entries == NULL) {
                fclose(fp);
                return 1;
            }
        }
        
        if (entries->recID > recID) {
            continue;
        } 
        
        if (entries->recID == recID) {
            // Pname = col 4
            strtok(NULL, "\t");
            col = strtok(NULL, "\t");
            stripped_word_copy(name, col);
            len = strlen(name);

            entries->fname = malloc(len+1);
            strcpy(entries->fname, name);

            // Sname = col 6
            strtok(NULL, "\t");
            col = strtok(NULL, "\t");
            stripped_word_copy(name, col);
            len = strlen(name);

            entries->lname = malloc(len+1);
            strcpy(entries->lname, name);
            
            entries = entries->next;
        } 
    }

    fclose(fp);
    return 0;
} // add_names

/* Copy first word without leading and trailing spaces. */
void stripped_word_copy(char *dest, char *src) {
    unsigned int flag=0; // copy only first word

    while (*src) {
        if (*src != ' ') {
            *dest++ = *src;
        } else {
            if (flag == 0) flag++;
            else break;
        }

        src++;
    }
    *dest = '\0';
}

/* Standardize first names. */
void standardize_fnames(entry_t *entries, name_dict_t *name_dict) {
    name_dict_t *head = name_dict;

    while (entries->next) {
        entries = entries->next;
        name_dict = head->next;

        // no name
        if (entries->fname == NULL) continue;

        // lookup by first letter
        while (name_dict && (entries->fname[0] > name_dict->fname[0])) {
            name_dict = name_dict->next;
        }

        // find full match with same first letter
        while (name_dict && (entries->fname[0] == name_dict->fname[0])) {
            if (strcmp(entries->fname, name_dict->fname) == 0) {
                // copy standardized fname
                int len = strlen(name_dict->fname_std);
                if ((entries->fname = realloc(entries->fname, len+1)) == NULL) break;
                strcpy(entries->fname, name_dict->fname_std);

                break; 
            }
            name_dict = name_dict->next;
        }
    }
    
    return;
} // standardize_fnames

/* Find matches between two entry lists using age and JW distance. */
match_t *find_matches(entry_t *entries_1851, entry_t *entries_1881) {
    match_t *ret, *cur_ret;
    entry_t *cur_1851, *cur_1881;
    
    cur_ret = ret = malloc(sizeof(match_t));
    ret->entry_1851 = NULL; ret->entry_1881 = NULL;
    ret->next = NULL;

    cur_1851 = entries_1851;
    cur_1881 = entries_1881;

    // for each 1851 entry
    while (cur_1851->next) {
        cur_1851 = cur_1851->next;

        // check each 1881 entry for satisfiability
        while (cur_1881->next) {
            cur_1881 = cur_1881->next;

            // age criteria
            if (cur_1851->age + 30 - cur_1881->age > 3) continue;

            // jarowinkler criteria
            if (!cur_1851->fname || !cur_1881->fname) continue;
            if (1-jarowinkler(cur_1851->fname, cur_1881->fname) > 0.2) continue;
            if (!cur_1851->lname || !cur_1881->lname) continue;
            if (1-jarowinkler(cur_1851->lname, cur_1881->lname) > 0.2) continue;
            if (!cur_1851->par || !cur_1881->par) continue;
            if (1-jarowinkler(cur_1851->par, cur_1881->par) > 0.2) continue;
            
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

/* Free entry_t list. */
void free_entries(entry_t *entries) {
    entry_t *tmp;

    while (entries) {
        tmp = entries;
        free(entries->fname);
        free(entries->lname);
        free(entries->par);

        entries = entries->next;
        free(tmp);
    }
    entries = NULL;
}

/* Free match_t list. */
void free_matches(match_t *matches) {
    match_t *tmp;

    while (matches) {
        tmp = matches;

        matches = matches->next;
        free(tmp);
    }
    matches = NULL;
}

#ifdef PRINT
/* Print the contents of an entry list. */
void print_entries(entry_t *entries) {
    int count=0;

    while (entries->next) {
        entries = entries->next;
        printf("%d %s %s %c %d %s\n", entries->recID, entries->fname, entries->lname,
                                      entries->sex, entries->age, entries->par);
        count++;
    }
    fprintf(stderr, "There were %d extracted entries.\n\n", count);
} // print_entries
#endif

/* Print the contents of a match list. */
void print_matches(match_t *matches) {
    int count=0;

    while (matches->next) {
        matches = matches->next;
        printf("%d %s %s %c %d %s\t-->\t", matches->entry_1851->recID, 
                matches->entry_1851->fname, matches->entry_1851->lname,
                matches->entry_1851->sex, matches->entry_1851->age,
                matches->entry_1851->par);
        printf("%d %s %s %c %d %s\n", matches->entry_1881->recID,
                matches->entry_1881->fname, matches->entry_1881->lname,
                matches->entry_1881->sex, matches->entry_1881->age,
                matches->entry_1881->par);
        count++;
    }
    fprintf(stderr, "There were %d extracted matches.\n", count);
} // print_matches
