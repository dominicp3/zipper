#include "decompress.h"

int main(int argc, char **argv)
{
        if (argc != 3) {
                printf("%s <compressed> <decompressed>\n", argv[0]);
                return 0;
        }

        struct c_byte c_byte[256];
        for (int i = 0; i < 256; i++) {
                c_byte[i].bits = calloc(32, 1);
                c_byte[i].num_bits = 0;
        }

        get_bitcodes(argv[1], c_byte);
        struct node *root = build_tree(c_byte);

        decompress(argv[1], argv[2], root);

        destroy_tree(root);
        destroy_c_byte(c_byte);
}

void get_bitcodes(char *huffman, struct c_byte c_byte[256])
{
        FILE *file = fopen(huffman, "rb");
        if (file == NULL)
                return;

        uint32_t dict_size;
        fread(&dict_size, 4, 1, file);

        uint8_t *file_bytes = malloc(dict_size);
        fread(file_bytes, dict_size, 1, file);
        fclose(file);

        uint8_t ptr = 0;
        long num_bits = 8 * (dict_size - 1) - file_bytes[dict_size - 1];

        for (long i = 0; i < num_bits;) {
                for (int j = 0; j < 8 && (i + j) < num_bits; j++) {
                        uint8_t bit = (file_bytes[(i + j) / 8] >> (7 - ((i + j) % 8))) & 1;
                        c_byte[ptr].num_bits |= bit << (7 - j);
                }

                i += 8;
                if (i >= num_bits)
                        break;

                for (uint8_t j = 0; j < c_byte[ptr].num_bits && (i + j) < num_bits; j++) {
                        uint8_t bit = (file_bytes[(i + j) / 8] >> (7 - ((i + j) % 8))) & 1;
                        c_byte[ptr].bits[j / 8] |= bit << (7 - (j % 8));
                }

                i += c_byte[ptr].num_bits;
                ptr++;
        }

        free(file_bytes);
}

struct node *build_tree(struct c_byte c_byte[256])
{
        struct node *root = malloc(sizeof *root);
        root->zero = NULL;
        root->one = NULL;

        struct node *current;

        for (short i = 0; i < 256; i++) {

                current = root;

                for (short j = 0; j < c_byte[i].num_bits; j++) {

                        uint8_t bit = (c_byte[i].bits[j / 8] >> (7 - (j % 8))) & 1;

                        struct node **child = bit ? &current->one : &current->zero;

                        if (!*child) {
                                *child = malloc(sizeof **child);
                                **child = (struct node) {.byte = i, .zero = NULL, .one = NULL};
                        }

                        current = *child;
                }
        }

        return root;
}

void decompress(char *f_in, char *f_out, struct node *root)
{
        FILE *in = fopen(f_in, "rb");
        FILE *out = fopen(f_out, "wb");

        struct stat st;
        fstat(fileno(in), &st);

        uint32_t dict_size;
        fread(&dict_size, 4, 1, in);
        fseek(in, dict_size, SEEK_CUR);

        long file_size = st.st_size - dict_size - 4;

        uint8_t *file_bytes = malloc(file_size);
        fread(file_bytes, file_size, 1, in);
        fclose(in);

        long num_bits = 8 * (file_size - 1) - file_bytes[file_size - 1];

        struct node *current = root;
        for (long i = 0; i < num_bits; i++) {

                uint8_t bit = (file_bytes[i / 8] >> (7 - (i % 8))) & 1;
                current = bit ? current->one : current->zero;

                if (current->zero == NULL && current->one == NULL) {
                        fwrite(&current->byte, 1, 1, out);
                        current = root;
                }
        }

        free(file_bytes);

        fclose(out);
}

void destroy_tree(struct node *n)
{
        if (n == NULL)
                return;
        destroy_tree(n->zero);
        destroy_tree(n->one);
        free(n);
}

void destroy_c_byte(struct c_byte c_byte[256])
{
        for (int i = 0; i < 256; i++)
                free(c_byte[i].bits);
}
