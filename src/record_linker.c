#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_linker.h"
#include "jarowinkler.h"
#include "name_dict.h"

int main(int argc, char *argv[]) {
    entry_t *entries_1851, *entries_1881;
    match_t *matches;
    int rc;

    // Check arguments
    if (argc != 6) {
        fprintf(stderr, "usage: %s data_1851 data_1881 names_1851 names_1881 "
                        "fnames_std\n", argv[0]);
        return -1;
    }

    // Extract valid entries from data files
    entries_1851 = extract_valid_entries(argv[1], 1851);
    entries_1881 = extract_valid_entries(argv[2], 1881);

    if (!entries_1851 || !entries_1881) exit_with_error("could not extract valid entries");

    // Add names to valid entries
    rc = add_names(argv[3], entries_1851);
    if (rc == -1) exit_with_error("could not add names to entries");
    rc = add_names(argv[4], entries_1881);
    if (rc == -1) exit_with_error("could not add names to entries");

    // Standardize fnames
    rc = standardize_fnames(argv[5], entries_1851);
    if (rc == -1) exit_with_error("could not standardize fnames");
    rc = standardize_fnames(argv[5], entries_1881);
    if (rc == -1) exit_with_error("could not standardize fnames");

#ifdef PRINT
    // Print valid entries from each list
    printf("Printing valid entries from file 1:\n");
    print_entries(entries_1851);
    printf("Printing valid entries from file 2:\n");
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
        }
        
        if (entries->recID > recID) {
            continue;
        } 
        
        if (entries->recID == recID) {
            // Pname = col 4
            strtok(NULL, "\t");
            col = strtok(NULL, "\t");
            stripped_strcpy(name, col);
            len = strlen(name);

            entries->fname = malloc(len+1);
            strncpy(entries->fname, name, len+1);

            // Sname = col 6
            strtok(NULL, "\t");
            col = strtok(NULL, "\t");
            stripped_strcpy(name, col);
            len = strlen(name);

            entries->lname = malloc(len+1);
            strncpy(entries->lname, name, len+1);
            
            entries = entries->next;
        } 
    }

    fclose(fp);
    return 0;
} // add_names

/* Copy string without leading and trailing spaces. */
void stripped_strcpy(char *dest, char *src) {
    while (*src) {
        if (*src != ' ') {
            *dest++ = *src;
        }
        src++;
    }
    *dest = '\0';
}

/* Standardize first names. */
int standardize_fnames(char *filename, entry_t *entries) {
    name_dict_t *name_dict;

    name_dict = generate_name_dict(filename);
    if (name_dict == NULL) return -1;

#ifdef PRINT
    static int once=0;
    if (!once) {
        printf("Printing standardized name dictionary:\n");
        print_name_dict(name_dict);
        once++;
    }
#endif

    while (entries->next) {
        entries = entries->next;
        standardize(entries, name_dict);
    }
    
    return 0;
} // standardize_fnames

/* Standardizes first name in one entry. */
void standardize(entry_t *entry, name_dict_t *name_dict) {
    // no name
    if (entry->fname == NULL) return;

    // lookup by first letter
    name_dict = name_dict->next;
    while (name_dict && (entry->fname[0] > name_dict->fname[0])) {
        name_dict = name_dict->next;
    }

    // find full match
    while (name_dict && (entry->fname[0] == name_dict->fname[0])) {
        if (strcmp(entry->fname, name_dict->fname) == 0) {
            // copy standardized fname
            int len = strlen(name_dict->fname_std);
            if (realloc(entry->fname, len+1) == NULL) return;
            strcpy(entry->fname, name_dict->fname_std);

            return; 
        }
        name_dict = name_dict->next;
    }

    // no match
    return;
}

/* Find matches between two entry lists using age and JW distance. */
match_t *find_matches(entry_t *entries_1851, entry_t *entries_1881) {
    match_t *ret, *cur_ret;
    entry_t *cur_1851, *cur_1881;
    
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

/* Exit with an error message. */
void exit_with_error(char *msg) {
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, msg);
    exit(-1);
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
    printf("There were %d extracted entries.\n\n", count);
} // print_entries

/* Print the contents of a match list. */
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
