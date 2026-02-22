#include "compressor.h"

int run()
{
    char input1[] = "ABABABABABABABAB";
    char input2[] = "HELLO HELLO HELLO HELLO";
    char input3[] = "AAAAAAAAAAAAAAAAAAAA";
    char input4[] = "The quick brown fox jumps over the lazy dog";

    char compressed[8192];
    char decompressed[8192];

    char* tests[] = { input1, input2, input3, input4 };
    int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < num_tests; i++)
    {
        printf("==== TEST %d ====\n", i + 1);
        printf("Original:     \"%s\"\n", tests[i]);

        int original_bytes = strlen(tests[i]);
        int original_bits = original_bytes * 8;

        compress_string(tests[i], compressed);
        int compressed_bytes = strlen(compressed);

        printf("Compressed:   %s\n", compressed);

        char compressed_copy[8192];
        strcpy(compressed_copy, compressed);

        decompress_string(compressed_copy, decompressed);
        printf("Decompressed: \"%s\"\n", decompressed);

        printf("Result: %s\n",
            strcmp(tests[i], decompressed) == 0 ? "PASS" : "FAIL");

        int total_bits = 0;
        char parse_buf[8192];
        strcpy(parse_buf, compressed);

        char* token = strtok(parse_buf, " ");
        while (token)
        {
            if (strcmp(token, "1") == 0)
            {
                strtok(NULL, " ");
                strtok(NULL, " ");
                total_bits += 18;
            }
            else
            {
                total_bits += 9;
            }
            token = strtok(NULL, " ");
        }

        double theoretical_factor = (double)total_bits / original_bits * 100.0;
        double ascii_factor = (double)compressed_bytes / original_bytes * 100.0;

        printf("Theoretical size: %.2f%% of original\n", theoretical_factor);

        printf("Original size:   %d bytes\n", original_bytes);
        printf("Compressed size: %d bytes\n", compressed_bytes);

        printf("\n");
    }

    return 0;
}
