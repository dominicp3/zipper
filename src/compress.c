#include "compress.h"
#define BUF_SIZE 100000000 // 100MB
#define MIN(x, y) x < y ? x : y

int main(int argc, char **argv)
{
        if (argc != 3) {
                printf("%s <filename> <dir>\n", argv[0]);
                return 0;
        }

        struct node **node = malloc(256 * sizeof *node);
        int num_nodes = set_value(argv[1], node);
        if (num_nodes == -1) {
                perror("");
                return 0;
        }
        struct node *root = build_tree(node, num_nodes); 

        struct c_byte c_byte[256];
        for (int i = 0; i < 256; i++) {
                c_byte[i].bits = NULL;
                c_byte[i].num_bits = 0;
        }

        search_tree(root, c_byte, 0);
        FILE *file = write_dictionary(argv[2], c_byte);
        write_compressed(argv[1], file, c_byte);

        destroy_tree(root);
        free(node);

        return 0;
}

int set_value(char *filename, struct node *node[256])
{
        FILE *file = fopen(filename, "rb");
        if (file == NULL)
                return -1;

        for (short i = 0; i < 256; i++)
                node[i] = NULL;

        struct stat st;
        fstat(fileno(file), &st);

        int num_nodes = 0;

        uint8_t *byte = malloc(BUF_SIZE);

        long offset = 0;
        while (offset < st.st_size) {
                size_t size = MIN(st.st_size, BUF_SIZE);
                fread(byte, size, 1, file);
                for (size_t i = 0; i < size; i++) {
                        if (node[byte[i]] == NULL) {
                                node[byte[i]] = node_init(byte[i]);
                                num_nodes++;
                        }
                        node[byte[i]]->value++;
                }
                offset += size;
        }

        fclose(file);
        free(byte);

        return num_nodes;
}

struct node *build_tree(struct node *node[256], int num_nodes)
{
        struct node *smallest[2];
        struct node *result = NULL;

        if (num_nodes == 1)
                for (int i = 0; i < 256; i++)
                        if (node[i] != NULL)
                                return node[i];
                                

        for (int i = 0; i < num_nodes - 1; i++) {
                int index = two_smallest(node, smallest);

                struct node *parent = node_init(-1);
                parent->zero = smallest[0];
                parent->one = smallest[1];
                parent->value = parent->zero->value + parent->one->value;

                node[index] = parent;
                result = parent;
        }

        return result;
}

void search_tree(struct node *n, struct c_byte c_byte[256], int depth)
{
        if (n->zero == NULL && n->one == NULL) {
                c_byte[n->byte].bits = n->bits;
                c_byte[n->byte].num_bits = depth;
                return;
        }

        memcpy(n->zero->bits, n->bits, 32);
        memcpy(n->one->bits, n->bits, 32);

        n->one->bits[depth / 8] |= (1 << (7 - (depth % 8)));        

        search_tree(n->zero, c_byte, depth + 1);
        search_tree(n->one, c_byte, depth + 1);
}

FILE *write_dictionary(char *dir, struct c_byte c_byte[256])
{
        FILE *file = fopen(dir, "wb");

        size_t size = 1024;
        uint8_t *arr = malloc(size);

        uint32_t byte_count = 0;
        size_t bit_count = 0;

        uint8_t byte = 0;
        for (int i = 0; i < 256; i++) {

                for (int j = 0; j < 8; j++) {
                        uint8_t bit = (c_byte[i].num_bits >> (7 - j)) & 1;
                        byte |= bit << (7 - (bit_count % 8));
                        arr[byte_count] = byte;

                        bit_count++;
                        if (bit_count % 8 == 0) {
                                byte = 0;
                                byte_count++;
                        }

                        if (byte_count >= size) {
                                arr = realloc(arr, 2 * size);
                                size *= 2;
                        }
                }

                for (int j = 0; j < c_byte[i].num_bits; j++) {
                        uint8_t bit = (c_byte[i].bits[j / 8] >> (7 - (j % 8))) & 1;
                        byte |= bit << (7 - (bit_count % 8));

                        arr[byte_count] = byte;

                        bit_count++;
                        if (bit_count % 8 == 0) {
                                byte = 0;
                                byte_count++;
                        }

                        if (byte_count >= size) {
                                arr = realloc(arr, 2 * size);
                                size *= 2;
                        }
                }
        }

        if (bit_count % 8 != 0)
                byte_count++;

        uint32_t file_size = byte_count + 1;
        uint8_t padding = (uint8_t) ((8 * byte_count) - bit_count);
        fwrite(&file_size, 4, 1, file);
        fwrite(arr, byte_count, 1, file);
        fwrite(&padding, 1, 1, file);;

        free(arr);

        return file;
}

void write_compressed(char *file_in, FILE *file_out, struct c_byte c_byte[256])
{
        FILE *in = fopen(file_in, "rb");
        if (in == NULL)
                return;

        struct stat in_st;
        fstat(fileno(in), &in_st);

        size_t size = 1024;

        uint8_t *in_arr = malloc(in_st.st_size);
        uint8_t *out_arr = malloc(size);

        fread(in_arr, in_st.st_size, 1, in);

        uint8_t byte_out = 0;
        long bit_count = 0;
        size_t byte_count = 0;

        for (long i = 0; i < in_st.st_size; i++) {

                uint8_t byte_in = in_arr[i];

                for (int j = 0; j < c_byte[byte_in].num_bits; j++) {
                        uint8_t bit = (c_byte[byte_in].bits[j / 8] >> (7 - (j % 8))) & 1;

                        byte_out |= bit << (7 - (bit_count % 8));
                        out_arr[byte_count] = byte_out;

                        bit_count++;
                        if (bit_count % 8 == 0) {
                                byte_out = 0;
                                byte_count++;
                        }    

                        if (byte_count >= size) {
                                out_arr = realloc(out_arr, 2 * size);
                                size *= 2;
                        }
                }
        }

        if (bit_count % 8 != 0)
                byte_count++;

        uint8_t padding = (uint8_t) ((8 * byte_count) - bit_count);

        fwrite(out_arr, byte_count, 1, file_out);
        fwrite(&padding, 1, 1, file_out);

        fclose(in);
        fclose(file_out);

        free(in_arr);
        free(out_arr);
}

void destroy_tree(struct node *n)
{
        if (n == NULL)
                return;
        destroy_tree(n->zero);
        destroy_tree(n->one);
        free(n);
}

struct node *node_init(int byte)
{
        struct node *n = calloc(1, sizeof *n);
        n->byte = byte;
        n->zero = NULL;
        n->one = NULL;
        return n;
}

int two_smallest(struct node *node[256], struct node *smallest[2])
{
        uint64_t min = UINT64_MAX;

        struct node *n1 = NULL;
        struct node *n2 = NULL;

        int index1 = -1;
        int index2 = -1;

        for (int i = 0; i < 256; i++) {
                if (node[i] && node[i]->value < min) {
                        n1 = node[i];
                        index1 = i;
                        min = node[i]->value;
                }
        }

        min = UINT64_MAX;

        for (int i = 0; i < 256; i++) {
                if (node[i] && i != index1 && node[i]->value < min) {
                        n2 = node[i];
                        index2 = i;
                        min = node[i]->value;
                }
        }

        smallest[0] = n1;
        smallest[1] = n2;

        node[index1] = NULL;
        node[index2] = NULL;

        return index1;
}

void print_bits(struct c_byte *c_byte)
{
        for (short i = 0; i < c_byte->num_bits; i++) {
                uint8_t bit = (c_byte->bits[i / 8] >> (7 - (i % 8))) & 1;
                printf("%d", bit);
        }
        printf("\n");
}