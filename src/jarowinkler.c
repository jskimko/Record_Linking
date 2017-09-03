#include <stdlib.h>
#include <string.h>
#include "jarowinkler.h"

/* Compute the Jaro distance between strings. */
/* jaro() adapted from https://rosettacode.org/wiki/Jaro_distance#C */
double jaro(const char *str1, const char *str2) {
    int len1, len2;                 // string lengths
    int match_distance;             // max distance for matching criteria
    int *matches1, *matches2;       // bool arrays for string matches
    double n_matches, n_trans;      // number of matches and transpositions
    int i,j;
    int start, end;

    len1 = strlen(str1);
    len2 = strlen(str2);

    // if both strings are empty return 1
    // if only one of the strings is empty return 0
    if (len1 == 0) return len2 == 0 ? 1.0 : 0.0;

    // max distance between two chars to be considered matching
    // floor() is ommitted due to integer division rules
    match_distance = (len1 > len2) ? len1/2 - 1 : len2/2 - 1;

    // arrays of bools that signify if that char in the matching string has a match
    matches1 = calloc(len1, sizeof(int));
    matches2 = calloc(len2, sizeof(int));

    // number of matches and transpositions
    n_matches = 0.0;
    n_trans = 0.0;

    // find the matches
    for (i = 0; i < len1; i++) {
        // start and end take into account the match distance
        start = (i - match_distance > 0) ? i - match_distance : 0;
        end = (i + match_distance + 1 < len2) ? i + match_distance + 1 : len2;

        for (j = start; j < end; j++) {
            // if str2 already has a match continue
            if (matches2[j]) continue;
            // if str1 and str2 are not
            if (str1[i] != str2[j]) continue;
            // otherwise assume there is a match
            matches1[i] = 1;
            matches2[j] = 1;
            n_matches++;
            break;
        }
    }

    // if there are no matches return 0
    if (n_matches == 0) {
        free(matches1);
        free(matches2);
        return 0.0;
    }

    // count transpositions
    j = 0;
    for (i = 0; i < len1; i++) {
        // if there are no matches in str1 continue
        if (!matches1[i]) continue;
        // while there is no match in str2 increment j
        while (!matches2[j]) j++;
        // increment transpositions
        if (str1[i] != str2[j]) n_trans++;
        j++;
    }

    // divide the number of transpositions by two as per the algorithm specs
    // this division is valid because the counted transpositions include both
    // instances of the transposed characters.
    n_trans /= 2.0;

    // free the allocated memory
    free(matches1);
    free(matches2);

    // return the Jaro distance
    return ((n_matches / len1) + (n_matches / len2) + 
           ((n_matches - n_trans) / n_matches)) / 3.0;
} // jaro

/* Compute the Jaro-Winkler distance between strings. */
double jarowinkler(const char *str1, const char *str2) {
    int l=0;                                // length of common prefix (max = 4)
    double p=0.1;                           // scaling factor
    double jaro_dist = jaro(str1, str2);    // jaro distance

    // count length of common prefix
    int i;
    for (i=0; i<4; i++) {
        // if end of string, stop
        if (!str1[i] || !str2[i]) break;
        // if chars match, increment l
        if (str1[i] == str2[i]) l = i+1;
        // else stop
        else break;
    }

    return jaro_dist + (l * p * (1 - jaro_dist));
} // jarowinkler

