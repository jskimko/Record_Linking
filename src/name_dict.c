#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "name_dict.h"

/* Generate sorted name dictionary. */
name_dict_t *generate_name_dict(char *filename) {
    FILE *fp;
    char buf[512];
    char *col;
    int len;
    name_dict_t *name_dict, *cur;

    // Open data file
    if ((fp = fopen(filename, "r")) == NULL) {
        perror("extract:fopen");
        return NULL;
    }

    // Remove header
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        perror("extract:fgets");
        fclose(fp);
        return NULL;
    }

    // Create name dictionary to map fname -> fname_std
    cur = name_dict = malloc(sizeof(name_dict_t));
    cur->next = NULL;
    cur->prev = NULL;

    while (fgets(buf, sizeof(buf), fp)) {
        // create new mapping
        name_dict_t *new_map = malloc(sizeof(name_dict_t));
        new_map->next = NULL;
        new_map->prev = cur;

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
    fclose(fp);

    sort_name_dict(name_dict);
    return name_dict;
}

/* Helper function to sort name dictionary. */
void sort_name_dict(name_dict_t *name_dict) {
    name_dict_t *cur, *swp;
    char *tmp;

    cur = name_dict->next->next;

    while (cur != NULL) {
        swp = cur;
        while ((swp->fname[0] < swp->prev->fname[0]) && (swp != name_dict)) {
            tmp = swp->prev->fname;
            swp->prev->fname = swp->fname;
            swp->fname = tmp;
            
            tmp = swp->prev->fname_std;
            swp->prev->fname_std = swp->fname_std;
            swp->fname_std = tmp;

            swp = swp->prev;
        }
        cur = cur->next;
    }
}

/* Print name dictionary. */
void print_name_dict(name_dict_t *name_dict) {
    name_dict_t *cur = name_dict;
    while (cur->next) {
        cur = cur->next;
        printf("%s %s\n", cur->fname, cur->fname_std);
    }
}
