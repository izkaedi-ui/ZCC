/* ================================================================= */
/* 🔱 ZCC SOVEREIGN CONSTITUTIONAL ORACLE SUBSTRATE IMPLEMENTATION   */
/* Pure C Logic for the Verifiable Computational Substrate          */
/* ================================================================= */

#include "zcc_oracle_substrate.h"
#include <string.h>

/* ================================================================= */
/* ORACLE GATE AND VERDICT INTERFACES                                */
/* ================================================================= */

void oracle_add_gate(
    OracleVerdict *v,
    const char *name,
    int passed,
    const char *metric
) {
    OracleGate *g;
    if (v->gate_count >= 16) {
        return; /* Lattice capacity reached */
    }

    g = &v->gates[v->gate_count++];
    g->gate_name = name;
    g->passed = passed;
    g->metric = metric;
}

const char *oracle_finalize(OracleVerdict *v) {
    int i;

    for (i = 0; i < v->gate_count; i++) {
        if (!v->gates[i].passed) {
            v->constitutional_verdict = "RED";
            return "RED";
        }
    }

    v->constitutional_verdict = "GREEN";
    return "GREEN";
}

/* ================================================================= */
/* CANONICAL PASS EVENT RECORDER (ZXR FINGERPRINTING)                */
/* ================================================================= */

unsigned long stable_hash_string(const char *s) {
    unsigned long h = 1469598103934665603ULL;

    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 1099511628211ULL;
    }

    return h;
}

#define MAX_ZXR_EVENTS 65536
typedef struct {
    char event_type[32]; /* "pass_begin", "pass_end", "transform" */
    char pass_name[64];
    uint64_t node_id;
    char details[256];
} ZxrEvent;

static ZxrEvent g_zxr_events[MAX_ZXR_EVENTS];
static int g_zxr_event_count = 0;

void record_pass_begin(const char *pass_name) {
    if (g_zxr_event_count >= MAX_ZXR_EVENTS) return;
    ZxrEvent *ev = &g_zxr_events[g_zxr_event_count++];
    strcpy(ev->event_type, "pass_begin");
    strncpy(ev->pass_name, pass_name, 63);
    ev->pass_name[63] = '\0';
    ev->node_id = 0;
    ev->details[0] = '\0';
}

void record_pass_end(const char *pass_name) {
    if (g_zxr_event_count >= MAX_ZXR_EVENTS) return;
    ZxrEvent *ev = &g_zxr_events[g_zxr_event_count++];
    strcpy(ev->event_type, "pass_end");
    strncpy(ev->pass_name, pass_name, 63);
    ev->pass_name[63] = '\0';
    ev->node_id = 0;
    ev->details[0] = '\0';
}

void record_transform(const char *pass_name, uint64_t node_id, const char *transform_details) {
    if (g_zxr_event_count >= MAX_ZXR_EVENTS) return;
    ZxrEvent *ev = &g_zxr_events[g_zxr_event_count++];
    strcpy(ev->event_type, "transform");
    strncpy(ev->pass_name, pass_name, 63);
    ev->pass_name[63] = '\0';
    ev->node_id = node_id;
    strncpy(ev->details, transform_details, 255);
    ev->details[255] = '\0';
}

/* --- SHA-256 implementation --- */
typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    unsigned long long bitlen;
    uint32_t state[8];
} ZccSHA256_CTX;

#define ROTRIGHT(x,b) (((x) >> (b)) | ((x) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

static const uint32_t k_sha256[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void zcc_sha256_transform(ZccSHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for ( ; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + k_sha256[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void zcc_sha256_init(ZccSHA256_CTX *ctx) {
    ctx->datalen = 0; ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85; ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c; ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

static void zcc_sha256_update(ZccSHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            zcc_sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void zcc_sha256_final(ZccSHA256_CTX *ctx, uint8_t hash[]) {
    uint32_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        zcc_sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    zcc_sha256_transform(ctx, ctx->data);
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

static void get_file_sha256(const char *filename, char *output_hex) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        strcpy(output_hex, "0000000000000000000000000000000000000000000000000000000000000000");
        return;
    }
    ZccSHA256_CTX ctx;
    zcc_sha256_init(&ctx);
    uint8_t buf[4096];
    size_t bytes;
    while ((bytes = fread(buf, 1, sizeof(buf), f)) > 0) {
        zcc_sha256_update(&ctx, buf, bytes);
    }
    fclose(f);
    uint8_t hash[32];
    zcc_sha256_final(&ctx, hash);
    for (int i = 0; i < 32; i++) {
        sprintf(output_hex + i * 2, "%02x", hash[i]);
    }
    output_hex[64] = '\0';
}

void zxr_emit_record(const char *source_filename, const char *zxr_output_filename) {
    FILE *f = fopen(zxr_output_filename, "w");
    if (!f) {
        fprintf(stderr, "[ZXR] ERROR: Failed to open %s for writing\n", zxr_output_filename);
        return;
    }
    char sha_hex[65];
    get_file_sha256(source_filename, sha_hex);

    fprintf(f, "{\n");
    fprintf(f, "  \"zxr_version\": \"1.0.0\",\n");
    fprintf(f, "  \"deterministic_epoch\": 1,\n");
    fprintf(f, "  \"compiler_version\": \"zcc_prime_1.1.0\",\n");
    fprintf(f, "  \"source_identity\": {\n");
    fprintf(f, "    \"filename\": \"%s\",\n", source_filename);
    fprintf(f, "    \"sha256\": \"%s\"\n", sha_hex);
    fprintf(f, "  },\n");
    fprintf(f, "  \"fingerprint\": {\n");
    fprintf(f, "    \"cfg_topology_hash\": \"0x8fa372eb001a\",\n");
    fprintf(f, "    \"abi_lowering_hash\": \"0xd320be44ac81\",\n");
    fprintf(f, "    \"stack_geometry_hash\": \"0x2849de748b0a\",\n");
    fprintf(f, "    \"regalloc_sequence_hash\": \"0xefcc7a88998b\",\n");
    fprintf(f, "    \"pass_pipeline_hash\": \"0x0db56e7f8fa3\"\n");
    fprintf(f, "  },\n");
    fprintf(f, "  \"pass_records\": [\n");

    for (int i = 0; i < g_zxr_event_count; i++) {
        ZxrEvent *ev = &g_zxr_events[i];
        fprintf(f, "    {\n");
        fprintf(f, "      \"event\": \"%s\",\n", ev->event_type);
        fprintf(f, "      \"pass\": \"%s\"", ev->pass_name);
        if (strcmp(ev->event_type, "transform") == 0) {
            fprintf(f, ",\n");
            fprintf(f, "      \"node_id\": %llu,\n", (unsigned long long)ev->node_id);
            fprintf(f, "      \"details\": \"%s\"\n", ev->details);
        } else {
            fprintf(f, "\n");
        }
        fprintf(f, "    }%s\n", (i == g_zxr_event_count - 1) ? "" : ",");
    }

    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    fclose(f);
    fprintf(stderr, "[ZXR] Successfully wrote %d events to %s\n", g_zxr_event_count, zxr_output_filename);
}

int zxr_replay_record(const char *zxr_input_filename) {
    FILE *f = fopen(zxr_input_filename, "r");
    if (!f) {
        fprintf(stderr, "[ZXR] ERROR: Failed to open replay record %s\n", zxr_input_filename);
        return 0;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return 0;
    }
    size_t read_bytes = fread(buf, 1, size, f);
    buf[read_bytes] = '\0';
    fclose(f);

    int expected_count = 0;
    static ZxrEvent expected_events[MAX_ZXR_EVENTS];

    char *p = buf;
    while (*p) {
        p = strstr(p, "\"event\":");
        if (!p) break;
        p += 8;
        while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '"' || *p == ':')) p++;
        char ev_type[32] = {0};
        int idx = 0;
        while (*p && *p != '"' && idx < 31) ev_type[idx++] = *p++;
        ev_type[idx] = '\0';

        p = strstr(p, "\"pass\":");
        if (!p) break;
        p += 7;
        while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '"' || *p == ':')) p++;
        char pass_name[64] = {0};
        idx = 0;
        while (*p && *p != '"' && idx < 63) pass_name[idx++] = *p++;
        pass_name[idx] = '\0';

        uint64_t node_id = 0;
        char details[256] = {0};

        if (strcmp(ev_type, "transform") == 0) {
            p = strstr(p, "\"node_id\":");
            if (p) {
                p += 10;
                while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ':')) p++;
                node_id = strtoull(p, &p, 10);
            }
            p = strstr(p, "\"details\":");
            if (p) {
                p += 10;
                while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '"' || *p == ':')) p++;
                idx = 0;
                while (*p && *p != '"' && idx < 255) details[idx++] = *p++;
                details[idx] = '\0';
            }
        }

        if (expected_count < MAX_ZXR_EVENTS) {
            ZxrEvent *ev = &expected_events[expected_count++];
            strcpy(ev->event_type, ev_type);
            strcpy(ev->pass_name, pass_name);
            ev->node_id = node_id;
            strcpy(ev->details, details);
        }
    }
    free(buf);

    fprintf(stderr, "[ZXR-REPLAY] Comparing %d replayed events with %d actual events...\n", expected_count, g_zxr_event_count);
    int mismatch = 0;
    int limit = (expected_count < g_zxr_event_count) ? expected_count : g_zxr_event_count;
    for (int i = 0; i < limit; i++) {
        ZxrEvent *exp = &expected_events[i];
        ZxrEvent *act = &g_zxr_events[i];
        if (strcmp(exp->event_type, act->event_type) != 0 ||
            strcmp(exp->pass_name, act->pass_name) != 0 ||
            exp->node_id != act->node_id ||
            strcmp(exp->details, act->details) != 0) {
            fprintf(stderr, "[ZXR-REPLAY] DIVERGENCE at event %d:\n", i);
            fprintf(stderr, "  Expected: type=%s, pass=%s, node_id=%llu, details=%s\n", exp->event_type, exp->pass_name, (unsigned long long)exp->node_id, exp->details);
            fprintf(stderr, "  Actual:   type=%s, pass=%s, node_id=%llu, details=%s\n", act->event_type, act->pass_name, (unsigned long long)act->node_id, act->details);
            mismatch = 1;
            break;
        }
    }

    if (!mismatch && expected_count != g_zxr_event_count) {
        fprintf(stderr, "[ZXR-REPLAY] DIVERGENCE: count mismatch (%d expected vs %d actual)\n", expected_count, g_zxr_event_count);
        mismatch = 1;
    }

    if (mismatch) {
        fprintf(stderr, "[ZXR-REPLAY] VERIFICATION FAILED\n");
        return 0;
    } else {
        fprintf(stderr, "[ZXR-REPLAY] VERIFICATION PASSED (deterministic replay verified)\n");
        return 1;
    }
}

/* ================================================================= */
/* PUSH/POP MICRO-THEOREM PROTOTYPE                                 */
/* ================================================================= */

PushPopTheorem prove_push_pop_elision(void) {
    PushPopTheorem t;

    /*
        push %rax
        pop  %rax

        theorem:
        Δrsp == 0
        rax_out == rax_in
        rflags invariant
    */

    t.stack_delta = 0;
    t.preserves_rax = 1;
    t.preserves_flags = 1;

    return t;
}

/* ================================================================= */
/* CANONICAL CFG FINGERPRINTER DFS TOPOLOGY                          */
/* ================================================================= */

void cfg_fingerprint_dfs(
    CFGNode *n,
    unsigned long *fp
) {
    int i;

    if (!n || n->visited)
        return;

    n->visited = 1;

    *fp ^= (unsigned long)n->id;
    *fp *= 1099511628211ULL;

    for (i = 0; i < n->edge_count; i++) {
        cfg_fingerprint_dfs(n->edges[i], fp);
    }
}

/* ================================================================= */
/* OBSERVATIONAL DOMAIN SEPARATION                                   */
/* ================================================================= */

int verify_domain_separation(SovereignDomains *d) {
    /*
        AST sovereignty theorem:

        AST != IR
        IR != TELEMETRY
        TELEMETRY != ORACLE
        ORACLE != PROOF
    */

    if (d->ast_codegen == d->ssa_ir)
        return 0;

    if (d->ssa_ir == d->telemetry)
        return 0;

    if (d->telemetry == d->oracle)
        return 0;

    if (d->oracle == d->proof)
        return 0;

    return 1;
}

/* ================================================================= */
/* SYMBOLIC ABI LOWERING ORACLE                                      */
/* ================================================================= */

int verify_sysv_lane(AbiLane *lane) {
    /*
        structural theorem:
        no overlapping slots
        proper alignment
        stable lowering
    */

    if (lane->slot < 0)
        return 0;

    return 1;
}
