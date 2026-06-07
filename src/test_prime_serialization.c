#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "prime_serialization.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_prime_state_c.pb>\n", argv[0]);
        return 1;
    }

    /* 1. Read binary file from disk */
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("Error opening input file");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *buffer = (uint8_t *)malloc(file_size);
    if (!buffer) {
        fprintf(stderr, "Out of memory allocating %ld bytes\n", file_size);
        fclose(f);
        return 1;
    }

    if (fread(buffer, 1, file_size, f) != (size_t)file_size) {
        perror("Error reading file");
        free(buffer);
        fclose(f);
        return 1;
    }
    fclose(f);

    printf("[C-TEST] Read binary payload size: %ld bytes\n", file_size);

    /* 2. Unpack binary struct */
    zcc_prime_consensus_t consensus;
    int unpack_res = zcc_prime_unpack_binary(buffer, file_size, &consensus);
    free(buffer);

    if (unpack_res != 0) {
        fprintf(stderr, "🔴 [C-TEST] Error unpacking binary consensus: %d\n", unpack_res);
        return 1;
    }

    printf("🟢 [C-TEST] Unpack Successful!\n");
    printf("           Consensus Score: %.6f\n", consensus.consensus_score);
    printf("           Consensus Drift: %.6f\n", consensus.drift);
    printf("           Jackpot Reward : %u\n", consensus.jackpot);
    printf("           Alerts         : %s\n", consensus.alerts);
    printf("           State H        : %.6f\n", consensus.state.h);
    printf("           State H0       : %.6f\n", consensus.state.h0);
    printf("           Context        : %s\n", consensus.state.context);
    printf("           History Count  : %lu\n", (unsigned long)consensus.state.history_count);

    /* 3. Serialize back to JSON and print */
    char json_buffer[4096];
    int serialize_res = zcc_prime_serialize_json(&consensus, json_buffer, sizeof(json_buffer));
    if (serialize_res != 0) {
        fprintf(stderr, "🔴 [C-TEST] Error serializing back to JSON\n");
        return 1;
    }

    printf("\n=== Re-serialized JSON output ===\n%s\n=================================\n", json_buffer);

    /* 4. Parse back from JSON and verify matching values */
    zcc_prime_consensus_t parsed;
    int deserialize_res = zcc_prime_deserialize_json(json_buffer, &parsed);
    if (deserialize_res != 0) {
        fprintf(stderr, "🔴 [C-TEST] Error deserializing from JSON\n");
        return 1;
    }

    if (fabs(parsed.consensus_score - consensus.consensus_score) < 1e-5 && parsed.jackpot == consensus.jackpot) {
        printf("🟢 [C-TEST] JSON Roundtrip verification: PASSED!\n");
    } else {
        fprintf(stderr, "🔴 [C-TEST] JSON Roundtrip verification: FAILED! (Parsed score: %.6f, original: %.6f)\n",
                parsed.consensus_score, consensus.consensus_score);
        return 1;
    }

    return 0;
}
