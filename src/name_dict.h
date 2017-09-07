#ifndef NAME_DICT_H
#define NAME_DICT_H

struct name_dict_t {
    char *fname;
    char *fname_std;
    struct name_dict_t *next;
    struct name_dict_t *prev;
};
typedef struct name_dict_t name_dict_t;

/* Generate sorted name dictionary. */
name_dict_t *generate_name_dict(char *filename);

/* Sort name dictionary. */
void sort_name_dict(name_dict_t *name_dict);

/* Free name_dict_t list. */
void free_name_dict(name_dict_t *name_dict);

#ifdef PRINT
/* Print name dictionary. */
void print_name_dict(name_dict_t *name_dict);
#endif

#endif
