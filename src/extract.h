#ifndef EXTRACT_H
#define EXTRACT_H

struct entry_t {
    unsigned int recID;
    char *fname, *lname;
    char *par;
    unsigned char age;
    char sex;
    struct entry_t *next;
};
typedef struct entry_t entry_t;

struct match_t {
    entry_t *entry_1851;
    entry_t *entry_1881;
    struct match_t *next;
};
typedef struct match_t match_t;

/* Extracts entries from data file where valid means male and below age 10. */
entry_t *extract_valid_entries(char *filename, int year);

/* Reads the name file and adds names to a list of valid entries. */
void *add_names(char *filename, entry_t *entries);

/* Find matches between two entry lists using age and JW distance. */
match_t *find_matches(entry_t *entries_1851, entry_t *entries_1881);

/* Compute the Jaro-Winkler distance between strings. */
double jarowinker(char *name_1851, char *name_1881);

/* Print the contents of an entry list. */
void print_entries(entry_t *entries);

#endif
