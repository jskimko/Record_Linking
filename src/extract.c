#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE *fp;
    char buf[4096];

    if (argc != 2) {
        fprintf(stderr, "usage: %s file\n", __FILE__, __LINE__, argv[0]);
        return -1;
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror("extract:fopen");
        return -1;
    }

    if (fgets(buf, sizeof(buf), fp) == NULL) {
        perror("extract:fgets");
        return -1;
    }

    printf("%s", buf);

    return 0;
}
