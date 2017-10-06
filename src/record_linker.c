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
    char *std_names;        // std_names filenames
    char *output;
    entry_t *entries1, *entries2;
    int count1, count2;
    name_dict_t *name_dict;
    match_t *matches;
    int rc=0;

#ifdef _OPENMP
    double wtime;
#endif

    // Parse arguments
    if (argc != 12) {
        fprintf(stderr, "Please use the run.sh script.\n");
        return -1;
    }

    data1 = argv[1]; data2 = argv[2];
    std_names = argv[3];
    year1 = atoi(argv[4]); year2 = atoi(argv[5]);
    sex_global = argv[6][0];
    min_age1 = atoi(argv[7]); max_age1 = atoi(argv[8]);
    min_age2 = atoi(argv[9]); max_age2 = atoi(argv[10]);
    output = argv[11];


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
            entries1 = extract_valid_entries(data1, &count1, 1);
            fprintf(stderr, "  Found %d valid entries in data 1\n", count1);
        }
        // Extract second data file
        #pragma omp task
        {
            entries2 = extract_valid_entries(data2, &count2, 2);
            fprintf(stderr, "  Found %d valid entries in data 2\n", count2);
        }
    }

#ifdef _OPENMP
    fprintf(stderr, "Took %lf seconds\n", omp_get_wtime() - wtime);
#endif

    if (!entries1 || !entries2) EXIT_WITH_ERROR("could not extract valid entries");


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
    fprintf(stderr, "Writing matches to output file '%s'...\n", output);

#ifdef _OPENMP
    wtime = omp_get_wtime();
#endif

    rc = write_matches(output, matches);
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
entry_t *extract_valid_entries(char *filename, int *count, int id) {
    FILE *fp;
    char buf[4096];
    char *col;
    entry_t *entries, *cur;
    int min_age, max_age;
    char *saveptr; // strtok_r

    unsigned int recID;
    char sex;
    unsigned char age;
    int len;

    // Open data file
    if ((fp = fopen(filename, "r")) == NULL) {
        perror("extract:fopen");
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

    cur = entries = malloc(sizeof(entry_t));
    cur->fname = cur->lname = NULL; cur->par = NULL;
    cur->next = NULL;
    *count = 0;

    //int n=0;
    // Store valid entries
    while (fgets(buf, sizeof(buf), fp)) {

        // RecID
        col = strtok_r(buf, ";", &saveptr);
        recID = atoi(col);

        // Sex
        col = strtok_r(NULL, ";", &saveptr);
        sex = col[0];
        if ((sex != sex_global) && (sex != 'U')) continue;

        // Age
        col = strtok_r(NULL, ";", &saveptr); 
        age = atoi(col);
        if ((age < min_age) || (age > max_age)) continue;

        // is valid entry
        entry_t *new_entry;
        new_entry = malloc(sizeof(entry_t));
        new_entry->recID = recID;
        new_entry->sex   = sex;
        new_entry->age   = age;
        new_entry->fname = NULL;
        new_entry->lname = NULL;
        new_entry->par   = NULL;
        new_entry->next  = NULL;

        // Std_Par
        col = strtok_r(NULL, ";", &saveptr);
        if (col[0] != '.') {
            len = strlen(col);
            new_entry->par = malloc(len+1);
            strcpy(new_entry->par, col);
        }

        // Pname
        col = strtok_r(NULL, ";", &saveptr);
        if (col[0] != '.') {
            len = strlen(col);
            new_entry->fname = malloc(len+1);
            strcpy(new_entry->fname, col);
        }

        // Sname
        col = strtok_r(NULL, ";", &saveptr);
        if (col[0] != '\n') {
            len = strlen(col);
            new_entry->lname = malloc(len);
            strcpy(new_entry->lname, col);
            new_entry->lname[len-1] = '\0';
        }

        cur->next = new_entry;
        cur = cur->next;
        (*count)++;
    }

    fclose(fp);
    return entries;
} // extract_valid_entries

/* Standardize first names. */
void standardize_fnames(entry_t *entries, name_dict_t *name_dict) {
    name_dict_t *head = name_dict;
    int len;

    // check each entry against name dictionary
    while (entries->next) {
        entries = entries->next;
        name_dict = head->next;

        // no name
        if (entries->fname == NULL) continue;

        while (name_dict) {
            if (strcmp(entries->fname, name_dict->fname) == 0) {
                // copy standardized fname
                len = strlen(name_dict->fname_std);
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
    entry_t *cur1, *cur2, *start_par;
    int diff = (year2 > year1) ? (year2-year1) : (year1-year2);
    
    cur_ret = ret = malloc(sizeof(match_t));
    ret->entry1 = NULL; ret->entry2 = NULL;
    ret->next = NULL;

    cur1 = entries1->next;
    cur2 = entries2->next;
    start_par = cur2;

#pragma omp parallel firstprivate(cur1, cur2, start_par)
{
#ifdef _OPENMP
    int n_threads = omp_get_num_threads();
    int tid = omp_get_thread_num();
    int iters = count / n_threads;
    int n=1, m=0;
    int i;

    for (i = 0; i < iters * tid; i++) {
        cur1 = cur1->next;
        m++;
    }

    #pragma omp single
    {
    fprintf(stderr, "  Using %d threads...\n", n_threads);
    }
    fprintf(stderr, "%d: starting at entry %d/%d\n", tid, m, count);
#endif

    // for each entry1
    while (cur1) {
#ifdef _OPENMP
        if (n % 10000 == 0) fprintf(stderr, "%d: iter %d/%d is %d\n", tid, n, iters, cur1->recID);
        if (n++ > iters) break;
#endif

        // go to parish
        while (cur2 && (strcmp(cur1->par, cur2->par) > 0)) {
            cur2 = cur2->next;
            start_par = cur2;
        }

        // check each entry2 with same parish
        while (cur2 && (strcmp(cur1->par, cur2->par) == 0)) {

            // age criteria
            if ((cur1->age + diff - cur2->age) > 3) {
                cur2 = cur2->next;
                continue;
            }

            // check that strings exist
            if (!cur1->fname || !cur2->fname || 
                !cur1->lname || !cur2->lname ||
                !cur1->par   || !cur2->par) {
                cur2 = cur2->next;
                continue;
            }

            // jarowinkler criteria
            if ((1-jarowinkler(cur1->fname, cur2->fname) > 0.2) || 
                (1-jarowinkler(cur1->lname, cur2->lname) > 0.2) || 
                (1-jarowinkler(  cur1->par,   cur2->par) > 0.2)) {
                cur2 = cur2->next;
                continue;
            }

//fprintf(stderr, "%d %s %s %c %d %s MATCHES %d %s %s %c %d %s\n", cur1->recID, cur1->fname, cur1->lname, cur1->sex, cur1->age, cur1->par, cur2->recID, cur2->fname, cur2->lname, cur2->sex, cur2->age, cur2->par);
            
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

            cur2 = cur2->next;
        }

        // reset entry2 pointer to starting parish
        cur2 = start_par;

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
