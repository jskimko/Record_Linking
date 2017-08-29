#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE *fp;
    char buf[4096];

    if (argc != 2) {
        fprintf(stderr, "%s:%d:usage: %s file\n", __FILE__, __LINE__, argv[0]);
        return -1;
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "%s:%d:fopen: cannot open '%s'\n", __FILE__, __LINE__, argv[1]);
        return -1;
    }

    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fprintf(stderr, "%s:%d:fgets\n", __FILE__, __LINE__);
        return -1;
    }

    printf("%s", buf);

    return 0;
}
