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

void print_entry(entry_t *entry);
entry_t *extract_valid_entries(char *filename, int year);

#endif
