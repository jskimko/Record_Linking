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
        return NULL;
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

/* Sort name dictionary. */
void sort_name_dict(name_dict_t *name_dict) {

}
