#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct entry_t {
    unsigned int recID;
    char *fname, *lname;
    char *par;
    unsigned char age;
    char sex;
    struct entry_t *next;
};
typedef struct entry_t entry_t;

void print_entry(entry_t *entry) {
    printf("%d %c %d %s\n", entry->recID, entry->sex, entry->age, entry->par);
}

int main(int argc, char *argv[]) {
    FILE *fp;
    char buf[4096];
    char *col;
    int n_lines=0, count=0;
    int i,j;
    entry_t *entries, *cur;

    // Check arguments
    if (argc != 2) {
        fprintf(stderr, "usage: %s file\n", __FILE__, __LINE__, argv[0]);
        return -1;
    }

    // Open data file
    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror("extract:fopen");
        return -1;
    }

    // Remove header
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        perror("extract:fgets");
        return -1;
    }

    // Count number of entries
    while (fgets(buf, sizeof(buf), fp)) n_lines++;

    // Go back to top of file and remove header
    fseek(fp, 0, SEEK_SET);
    fgets(buf, sizeof(buf), fp);

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
        col = strtok(NULL, "\t"); 
        age = atoi(col);
        if (age > 10) continue;

        // Std_Par = col 69
        for (j=0; j<24; j++) strtok(NULL, "\t");
        col = strtok(NULL, "\t");
        len = strlen(col);

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

    return 0;
}
