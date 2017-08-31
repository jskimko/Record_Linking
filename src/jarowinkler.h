#ifndef JAROWINKLER_H
#define JAROWINKLER_H

/* Compute the Jaro distance between strings. */
double jaro(const char *str1, const char *str2);

/* Compute the Jaro-Winkler distance between strings. */
double jarowinkler(const char *str1, const char *str2);

#endif
