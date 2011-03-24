#include "btree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <time.h>

static int file_exists(const char *path)
{
        struct stat st;
        return stat(path, &st) == 0;
}

static struct timespec start;

static void start_timer(void)
{
        clock_gettime(CLOCK_MONOTONIC, &start);
}

static double get_timer(void)
{
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        long seconds  = end.tv_sec  - start.tv_sec;
        long nseconds = end.tv_nsec - start.tv_nsec;
        return seconds + (double) nseconds / 1.0e9;
}

#define COUNT		1000000

int main(int argc, char **argv)
{
	const char *fname = "test.dat";

	if (argc < 2)
		return 0;

	srand(time(NULL));

	struct btree btree;
	uint8_t sha1[SHA1_LENGTH];
	char val[100];
	size_t i;

	if (file_exists(fname)) {
		if (btree_open(&btree, fname)) {
			printf("Unable to open database\n");
			return 1;
		}
	} else {
		if (btree_creat(&btree, fname)) {
			printf("Unable to create database\n");
			return 1;
		}
	}

	if (strcmp(argv[1], "insert") == 0) {
		memset(sha1, 0, sizeof sha1);

		start_timer();
		for (i = 0; i < COUNT; ++i) {
			sprintf((char *) sha1, "foobar %zd", i);
			sprintf(val, "value %zd", i*i);
			btree_insert(&btree, sha1, val, strlen(val));
		}
		printf("insert: %.6f\n", get_timer());
	}

	if (strcmp(argv[1], "get") == 0) {
		memset(sha1, 0, sizeof sha1);
		strcpy((char *) sha1, "foobar ");

		start_timer();
		for (i = 0; i < COUNT; ++i) {
			sprintf((char *) sha1 + 7, "%zd", i);
			size_t len;
			void *data = btree_get(&btree, sha1, &len);
			if (data == NULL)
				printf("%zd %p,%zd\n", i, data, len);
			free(data);
		}
		printf("get: %.6f\n", get_timer());
	}

	if (strcmp(argv[1], "refill") == 0) {
		memset(sha1, 0, sizeof sha1);
		for (i = 0; i < COUNT/2; i++) {
			sprintf((char *) sha1, "foobar %zd", i);
			if (btree_delete(&btree, sha1))
				printf("DELETE %zd\n", i);
		}
		memset(sha1, 0, sizeof sha1);
		for (i = 0; i < COUNT/2; i++) {
			sprintf((char *) sha1, "foobar %zd", i);
			sprintf(val, "testingtestingtesting%zd", i*i);
			btree_insert(&btree, sha1, val, strlen(val));
		}
	}

	if (strcmp(argv[1], "delete") == 0) {
		memset(sha1, 0, sizeof sha1);

		start_timer();
		for (i = 0; i < COUNT; i++) {
			sprintf((char *) sha1, "foobar %zd", i);
			if (btree_delete(&btree, sha1))
				printf("DELETE %zd\n", i);
		}
		printf("delete: %.6f\n", get_timer());
	}

	btree_close(&btree);

	return 0;
}
