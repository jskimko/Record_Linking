#ifndef RECORD_LINKER_H
#define RECORD_LINKER_H
#include "name_dict.h"

/* Exit with an error message. */
#define EXIT_WITH_ERROR(msg) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, msg); exit(-1); }

/* Stores relevant information for one entry. */
struct entry_t {
    unsigned int recID;
    char *fname, *lname;
    char *par;
    unsigned char age;
    char sex;
    struct entry_t *next;
};
typedef struct entry_t entry_t;

/* Stores one potential match consisting of two entries. */
struct match_t {
    entry_t *entry1;
    entry_t *entry2;
    struct match_t *next;
};
typedef struct match_t match_t;

/* Extracts entries from data file based on sex and age. */
entry_t *extract_valid_entries(char *filename);

/* Reads the name file and adds names to a list of valid entries. */
int add_names(char *filename, entry_t *entries);

/* Copy first word without leading and trailing spaces. */
void stripped_word_copy(char *dest, char *src);

/* Standardize first names. */
void  standardize_fnames(entry_t *entries, name_dict_t *name_dict);

/* Find matches between two entry lists using age and JW distance. */
match_t *find_matches(entry_t *entries1, entry_t *entries2);

/* Free entry_t list. */
void free_entries(entry_t *entries);

/* Free match_t list. */
void free_matches(match_t *matches);

#ifdef PRINT
/* Print the contents of an entry list. */
void print_entries(entry_t *entries);
#endif

/* Print the contents of a match list. */
void print_matches(match_t *matches);

#endif
