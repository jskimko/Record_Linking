#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "record_linker.h"
#include "jarowinkler.h"
#include "name_dict.h"

unsigned char year1, year2;
unsigned char min_age1, max_age1;
unsigned char min_age2, max_age2;
char sex_global;

int main(int argc, char *argv[]) {
    char *data1, *data2;    // data filenames
    char *names1, *names2;  // names filenames
    char *std_names;        // std_names filenames
    char *filename;
    entry_t *entries1, *entries2;
    int count1, count2;
    name_dict_t *name_dict;
    match_t *matches;
    int rc=0;

#ifdef _OPENMP
    double wtime;
#endif

    // Parse arguments
    if (argc != 14) {
        //fprintf(stderr, "usage: %s data1 data2 names1 names2 std_names "
        //      "year1 year2 sex min_age1 max_age1 min_age2 max_age2\n", argv[0]);
        fprintf(stderr, "Please use the run.sh script.\n");
        return -1;
    }

    data1 = argv[1]; data2 = argv[2];
    names1 = argv[3]; names2 = argv[4];
    std_names = argv[5];
    year1 = atoi(argv[6]); year2 = atoi(argv[7]);
    sex_global = argv[8][0];
    min_age1 = atoi(argv[9]); max_age1 = atoi(argv[10]);
    min_age2 = atoi(argv[11]); max_age2 = atoi(argv[12]);
    filename = argv[13];


    /* Extract valid entries from data files */
    fprintf(stderr, "Extracting data...\n");

#ifdef _OPENMP
    wtime = omp_get_wtime();
#endif

    #pragma omp parallel
    #pragma omp single nowait
    {
        // Extract first data file
        #pragma omp task
        {
            entries1 = extract_valid_entries(data1, &count1);
            fprintf(stderr, "  Found %d valid entries in data 1\n", count1);
        }
        // Extract second data file
        #pragma omp task
        {
            entries2 = extract_valid_entries(data2, &count2);
            fprintf(stderr, "  Found %d valid entries in data 2\n", count2);
        }
    }

#ifdef _OPENMP
    fprintf(stderr, "Took %lf seconds\n", omp_get_wtime() - wtime);
#endif

    if (!entries1 || !entries2) EXIT_WITH_ERROR("could not extract valid entries");


    /* Add names to valid entries */
    fprintf(stderr, "Adding names...\n");

#ifdef _OPENMP
    wtime = omp_get_wtime();
#endif

    #pragma omp parallel private(rc)
    #pragma omp single nowait
    {
        // Add names to first entry list
        #pragma omp task
        {
            rc = add_names(names1, entries1);
            if (rc == -1) EXIT_WITH_ERROR("could not add names to entries");
            fprintf(stderr, "  Added names 1\n");
        }
        // Add names to second entry list
        #pragma omp task
        {
            rc = add_names(names2, entries2);
            if (rc == -1) EXIT_WITH_ERROR("could not add names to entries");
            fprintf(stderr, "  Added names 2\n");
        }
    }

#ifdef _OPENMP
    fprintf(stderr, "Took %lf seconds\n", omp_get_wtime() - wtime);
#endif


    /* Generate name dictionary */
    name_dict = generate_name_dict(std_names);
    if (name_dict == NULL) EXIT_WITH_ERROR("could not generate name dictionary");


    /* Standardize fnames */
    fprintf(stderr, "Standardizing names...\n");

#ifdef _OPENMP
    wtime = omp_get_wtime();
#endif

    #pragma omp parallel private(rc)
    #pragma omp single nowait
    {
        // Standardize names in first entry list
        #pragma omp task
        {
            standardize_fnames(entries1, name_dict);
            fprintf(stderr, "  Standardized names 1\n");
        }
        // Standardize names in second entry list
        #pragma omp task
        {
            standardize_fnames(entries2, name_dict);
            fprintf(stderr, "  Standardized names 2\n");
        }
    }

#ifdef _OPENMP
    fprintf(stderr, "Took %lf seconds\n", omp_get_wtime() - wtime);
#endif


    /* Free name dictionary */
    free_name_dict(name_dict);


#ifdef PRINT
    /* Print valid entries from each list */
    printf("Printing valid entries from file 1:\n");
    print_entries(entries1);
    printf("There were %d extracted entries.\n\n", count1);

    printf("Printing valid entries from file 2:\n");
    print_entries(entries2);
    printf("There were %d extracted entries.\n\n", count2);
#endif


    /* Find matches */
    fprintf(stderr, "Finding matches...\n");

#ifdef _OPENMP
    wtime = omp_get_wtime();
#endif

    matches = find_matches(entries1, entries2, count1);

#ifdef _OPENMP
    fprintf(stderr, "Took %lf seconds\n", omp_get_wtime() - wtime);
#endif


    /* Write matches */
    fprintf(stderr, "Writing matches to output file '%s'...\n", filename);

#ifdef _OPENMP
    wtime = omp_get_wtime();
#endif

    rc = write_matches(filename, matches);
    if (rc == -1) EXIT_WITH_ERROR("could not write output");

#ifdef _OPENMP
    fprintf(stderr, "Took %lf seconds\n", omp_get_wtime() - wtime);
#endif


    /* Free data */
    free_entries(entries1); free_entries(entries2);
    free_matches(matches);

    return 0;
} // main

/* Extracts entries from data file based on sex and age. */
entry_t *extract_valid_entries(char *filename, int *count) {
    FILE *fp;
    char buf[4096];
    char *col;
    int i;
    entry_t *entries, *cur;
    int min_age, max_age;
    char *saveptr; // strtok_r

    unsigned int recID;
    char sex;
    unsigned char age;
    int len;

    static int id = 1; // first or second call

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

    // Get age range
    if (id == 1) {
        min_age = min_age1;
        max_age = max_age1;
    } else if (id == 2) {
        min_age = min_age2;
        max_age = max_age2;
    } else {
        fprintf(stderr, "%s:%d: cannot read more than two files\n", 
                __FILE__, __LINE__);
        return NULL;
    } 
    id++;

    cur = entries = malloc(sizeof(entry_t));
    cur->fname = cur->lname = NULL; cur->par = NULL;
    cur->next = NULL;
    *count = 0;

    // Store valid entries
    while (fgets(buf, sizeof(buf), fp)) {
        // RecID = col 2
        strtok_r(buf, "\t", &saveptr);
        col = strtok_r(NULL, "\t", &saveptr);
        recID = atoi(col);

        // Sex = col 42
        for (i=0; i<39; i++) strtok_r(NULL, "\t", &saveptr);
        col = strtok_r(NULL, "\t", &saveptr);
        sex = col[0];
        if ((sex != sex_global) && (sex != 'U')) continue;

        // Age = col 44
        strtok_r(NULL, "\t", &saveptr);
        col = strtok_r(NULL, "\t", &saveptr); 
        age = atoi(col);
        if (age < min_age || age > max_age) continue;

        // Std_Par = col 69
        for (i=0; i<24; i++) strtok_r(NULL, "\t", &saveptr);
        col = strtok_r(NULL, "\t", &saveptr);
        len = strlen(col);

        // Save valid entry
        entry_t *new_entry;
        new_entry = malloc(sizeof(entry_t));
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
        (*count)++;
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
    char *saveptr; // strtok_r

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
        strtok_r(buf, "\t", &saveptr);
        col = strtok_r(NULL, "\t", &saveptr);
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
            strtok_r(NULL, "\t", &saveptr);
            col = strtok_r(NULL, "\t", &saveptr);
            stripped_word_copy(name, col);
            len = strlen(name);

            entries->fname = malloc(len+1);
            strcpy(entries->fname, name);

            // Sname = col 6
            strtok_r(NULL, "\t", &saveptr);
            col = strtok_r(NULL, "\t", &saveptr);
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
match_t *find_matches(entry_t *entries1, entry_t *entries2, int count) {
    match_t *ret, *cur_ret;
    entry_t *cur1, *cur2;
    int diff = (year2 > year1) ? (year2-year1) : (year1-year2);
    
    cur_ret = ret = malloc(sizeof(match_t));
    ret->entry2 = NULL; ret->entry2 = NULL;
    ret->next = NULL;

    cur1 = entries1->next;
    cur2 = entries2;

#pragma omp parallel firstprivate(cur1, cur2)
{
#ifdef _OPENMP
    int n_threads = omp_get_num_threads();
    int tid = omp_get_thread_num();
    int iters = count / n_threads;
    int i;

    int m = 0;
    for (i = 0; i < iters * tid; i++) {
        cur1 = cur1->next;
        m++;
    }

    #pragma omp single
    {
    fprintf(stderr, "  Using %d threads...\n", n_threads);
    }
    fprintf(stderr, "%d: starting at entry %d/%d\n", omp_get_thread_num(), m, count);
#endif

    int n=0;
    // for each entry1
    while (cur1) {
        fprintf(stderr, "%d: %d %d\n", omp_get_thread_num(), n, cur1->recID);
        n++;
        // check each entry2 for satisfiability
        while (cur2->next) {
            cur2 = cur2->next;

            // age criteria
            if ((cur1->age + diff - cur2->age) > 3) continue;

            // jarowinkler criteria
            if (!cur1->fname || !cur2->fname) continue;
            if (1-jarowinkler(cur1->fname, cur2->fname) > 0.2) continue;
            if (!cur1->lname || !cur2->lname) continue;
            if (1-jarowinkler(cur1->lname, cur2->lname) > 0.2) continue;
            if (!cur1->par || !cur2->par) continue;
            if (1-jarowinkler(cur1->par, cur2->par) > 0.2) continue;
            
            // save match
            match_t *new_match = malloc(sizeof(match_t));
            new_match->entry1 = cur1;
            new_match->entry2 = cur2;
            new_match->next = NULL;

            #pragma omp critical
            {
            cur_ret->next = new_match;
            cur_ret = cur_ret->next;
            }

        }

        // reset entry2 pointer
        cur2 = entries2;

        cur1 = cur1->next;
    }
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
    while (entries->next) {
        entries = entries->next;
        printf("%d %s %s %c %d %s\n", entries->recID, entries->fname, entries->lname,
                                      entries->sex, entries->age, entries->par);
        count++;
    }
} // print_entries
#endif

/* Print the contents of a match list. */
int write_matches(char *filename, match_t *matches) {
    FILE *fp;
    if ((fp = fopen(filename, "w")) == NULL) return -1;

    while (matches->next) {
        matches = matches->next;
        fprintf(fp, "%d %s %s %c %d %s\t-->\t", matches->entry1->recID, 
                matches->entry1->fname, matches->entry1->lname,
                matches->entry1->sex, matches->entry1->age,
                matches->entry1->par);
        fprintf(fp, "%d %s %s %c %d %s\n", matches->entry2->recID,
                matches->entry2->fname, matches->entry2->lname,
                matches->entry2->sex, matches->entry2->age,
                matches->entry2->par);
    }
    fclose(fp);

    return 0;
} // print_matches
