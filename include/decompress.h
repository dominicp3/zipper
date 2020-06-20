#ifndef DECOMPRESS_H
#define DECOMPRESS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "struct.h"

void get_bitcodes(char *huffman, struct c_byte c_byte[256]);
struct node *build_tree(struct c_byte c_byte[256]);
void decompress(char *f_in, char *f_out, struct node *root);
void destroy_tree(struct node *n);
void destroy_c_byte(struct c_byte c_byte[256]);

#endif /* DECOMPRESS_H */