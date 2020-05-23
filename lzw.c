//
//  Adam Patyk
//  lzw.c
//  ECE 6680 Lab 3: LZW Codec
//
//  Copyright © 2020 Adam Patyk. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DICT_MAX_SIZE 65536
#define MAX_CODE_LEN  512

void lzw_compress(FILE *, char *);
void lzw_decompress(FILE *, char *, int);
unsigned char **init_dictionary(void);
int *init_d_length(void);
int find_code(unsigned char **, int *, int, unsigned char *);
unsigned char *concat(unsigned char *, unsigned char *, unsigned char, int);
int add_to_dict(unsigned char **, int *, unsigned char *, int, int);
void print_dictionary(unsigned char **, int *);

int main(int argc, char *const *argv) {
    FILE    *fpt_in;
    int     c, len;

    if (argc != 3) {
        fprintf(stderr, "Usage: ./lzw -flag <file>\n");
        printf("Command line options\n");
        printf("Options -----------------\n");
        printf("  -c\tcompress file using LZW codec\n");
        printf("  -d\tdecompress file using LZW codec\n");
        exit(0);
    }

    // open file to be compressed
    fpt_in = fopen(argv[2], "rb");

    // command line argument handling
    while ((c = getopt(argc, argv, "cd")) != -1)
        switch (c) {
        case 'c': // compress
            printf("Compressing %s...\n", argv[2]);
            lzw_compress(fpt_in, argv[2]);
            break;

        case 'd': // decompress
            printf("Decompressing %s...\n", argv[2]);
            // check for .lzw extension
            len = strlen(argv[2]);

            if (strcmp(&argv[2][len - 4], ".lzw") != 0) {
                fprintf(stderr, "Must be an .lzw archive!\n");
                exit(0);
            }

            lzw_decompress(fpt_in, argv[2], len);
            break;

        default:
            printf("Command line options\n");
            printf("Options -----------------\n");
            printf("  -c\t\tcompress file using LZW codec\n");
            printf("  -d\t\tdecompress file using LZW codec\n");
            exit(1);
        }

    fclose(fpt_in);
    return 0;
}

void lzw_compress(FILE *fpt_in, char *filename) {
    unsigned char       c, *p, *buf, **dictionary;
    unsigned short int  code;
    int                 i, found, num_entries, p_length, *d_length;

    FILE *fpt_out = fopen(strcat(filename, ".lzw"), "wb");

    // initialize dictionary with all roots
    dictionary = init_dictionary();
    d_length = init_d_length();
    num_entries = 256;

    p = (unsigned char *)calloc(MAX_CODE_LEN, sizeof(unsigned char));
    buf = (unsigned char *)calloc(MAX_CODE_LEN, sizeof(unsigned char));
    p_length = 0;

    while (!feof(fpt_in)) {
        fread(&c, 1, 1, fpt_in);

        // store p + c
        buf = concat(buf, p, c, p_length);

        // check if p + c is in dictionary
        found = (find_code(dictionary, d_length, p_length + 1, buf) == -1) ? 0 : 1;

        if (!feof(fpt_in)) {
            if (found == 1) {
                // set p = p + c
                p_length++;

                for (i = 0; i < p_length; i++)
                    p[i] = buf[i];
            } else {
                // output code for p
                code = find_code(dictionary, d_length, p_length, p);
                fwrite(&code, 1, 2, fpt_out);

                // add p + c to dictionary
                num_entries = add_to_dict(dictionary, d_length, buf, p_length + 1, num_entries);

                // let p = c
                for (i = 0; i < p_length; i++)
                    p[i] = 0;

                p[0] = c;
                p_length = 1;
            }
        }
    }

    // output code for p
    code = find_code(dictionary, d_length, p_length, p);
    fwrite(&code, 1, 2, fpt_out);

    // cleanup
    for (i = 0; i < DICT_MAX_SIZE; i++)
        free(dictionary[i]);

    free(dictionary);
    free(d_length);
    free(p);
    free(buf);
    fclose(fpt_out);
}

void lzw_decompress(FILE *fpt_in, char *filename, int len) {
    FILE                *fpt_out;
    unsigned char       *x, y, *buf, **dictionary;
    unsigned short int  c, p;
    int                 i, found, num_entries, *d_length;

    buf = (unsigned char *)calloc(MAX_CODE_LEN, sizeof(unsigned char));
    x = (unsigned char *)calloc(MAX_CODE_LEN, sizeof(unsigned char));

    // open renamed output file
    char *new_name = (char *)calloc(len + 10, sizeof(char));

    // check for original extension
    if (filename[len - 8] == '.') {
        strncpy(new_name, filename, len - 8);
        // append "-recovered" to name
        strcpy(new_name + len - 8, "-recovered");
        // append original extension
        strncpy(new_name + len + 2, &filename[len - 8], 4);
    }
    // no extension (binary files)
    else {
        // append "-recovered" to name
        strncpy(new_name, filename, len - 4);
        strcpy(new_name + len - 4, "-recovered");
    }

    fpt_out = fopen(new_name, "wb");

    // initialize dictionary with all roots
    dictionary = init_dictionary();
    d_length = init_d_length();
    num_entries = 256;

    // find and output pattern for C
    fread(&c, 1, 2, fpt_in);
    fwrite(dictionary[c], 1, 1, fpt_out);

    while (!feof(fpt_in)) {
        // store previous (p) and read current (c)
        p = c;
        fread(&c, 1, 2, fpt_in);

        // check if c is in the dictionary
        found = (d_length[c] > 0) ? 1 : 0;

        if (!feof(fpt_in)) {
            if (found == 1) {
                // output code for c
                for (i = 0; i < d_length[c]; i++)
                    fwrite(&dictionary[c][i], 1, 1, fpt_out);

                // let x = pattern for p
                for (i = 0; i < d_length[p]; i++)
                    x[i] = dictionary[p][i];

                // let y = first char of pattern for c
                y = dictionary[c][0];

                // add x + y to dictionary
                buf = concat(buf, x, y, d_length[p]);
                num_entries = add_to_dict(dictionary, d_length, buf, d_length[p] + 1, num_entries);
            } else {
                // let x = pattern for p
                for (i = 0; i < d_length[p]; i++)
                    x[i] = dictionary[p][i];

                // let y = first char of pattern for p
                y = dictionary[p][0];

                // store x + y
                buf = concat(buf, x, y, d_length[p]);

                // output x + y
                for (i = 0; i < d_length[p]; i++)
                    fwrite(&x[i], 1, 1, fpt_out);

                fwrite(&y, 1, 1, fpt_out);

                // add x + y to dictionary
                num_entries = add_to_dict(dictionary, d_length, buf, d_length[p] + 1, num_entries);
            }
        }
    }

    // cleanup
    for (i = 0; i < DICT_MAX_SIZE; i++)
        free(dictionary[i]);

    free(dictionary);
    free(d_length);
    free(buf);
    free(x);
    free(new_name);
    fclose(fpt_out);
}

unsigned char **init_dictionary(void) {
    int i;
    
    // initialize dictionary with all roots
    unsigned char **dictionary = (unsigned char **)calloc(DICT_MAX_SIZE, sizeof(unsigned char *));

    for (i = 0; i < DICT_MAX_SIZE; i++)
        dictionary[i] = (unsigned char *)calloc(MAX_CODE_LEN, sizeof(unsigned char));

    // populate dictionary
    for (i = 0; i < 256; i++)
        dictionary[i][0] = (unsigned char) i;

    return dictionary;
}

int *init_d_length(void) {
    int i;
    int *d_length = (int *)calloc(DICT_MAX_SIZE, sizeof(int));

    // populate dictionary
    for (i = 0; i < 256; i++)
        d_length[i] = 1;

    return d_length;
}

int find_code(unsigned char **dictionary, int *d_length, int x_length, unsigned char *x) {
    int i, j, found, match, code;
    i = found = 0;
    code = -1; // if pattern is not found in dictionary

    while (d_length[i] > 0 && found == 0) {
        match = 0;

        // find p
        if (x_length == d_length[i]) {
            for (j = 0; j < d_length[i]; j++)
                if (x[j] ==  dictionary[i][j]) match++;

            if (match == d_length[i]) {
                found = 1;
                code = i;
            }
        }

        i++;
    }

    return code;
}

unsigned char *concat(unsigned char *dst, unsigned char *src1, unsigned char src2, int len) {
    int i;

    for (i = 0; i < len; i++)
        dst[i] = src1[i];

    dst[len] = src2;
    return dst;
}

int add_to_dict(unsigned char **dict, int *d_length, unsigned char *buf, int len, int num) {
    int i;

    for (i = 0; i < len; i++)
        dict[num][i] = buf[i];

    d_length[num] = len;
    return ++num;
}

void print_dictionary(unsigned char **dict, int *d_len) {
    int i = 0;
    printf("%5s | %-5s\n", "Code", "Symbol");
    printf("---------------\n");

    while (d_len[i] > 0) {
        printf("%5d | %-5s\n", i, dict[i]);
        i++;
    }
}
