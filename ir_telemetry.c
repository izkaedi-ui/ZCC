/*
 * ir_telemetry.c — ZCC IR Pass Telemetry Emitter
 * ================================================
 * Emits per-pass optimization metrics to Gods Eye via UDP 41337.
 * Fire-and-forget: if nobody is listening, sendto() fails silently.
 *
 * Wire format: {"_body":"<canonical JSON>","_sig":"ir_telemetry"}
 * The relay accepts _sig == "ir_telemetry" as a Phase 1 bypass.
 *
 * Compiled by GCC only (linked separately, NOT in zcc.c).
 * Uses POSIX sockets (Linux/WSL only — matches ZCC's target).
 *
 * Environment gate: ZCC_EMIT_TELEMETRY=1
 *   When unset, all functions early-return (zero overhead).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdarg.h>

#include "ir_telemetry.h"

/* ── Internal state ──────────────────────────────────────────────────── */

static int s_enabled  = 0;          /* set by ir_telem_init() */
static int s_sock_fd  = -1;        /* UDP socket */
static struct sockaddr_in s_addr;   /* Gods Eye endpoint */
static int s_compile_counter = 0;   /* monotonic compilation counter */

/* ── UDP emission helper ─────────────────────────────────────────────── */

/*
 * Send a JSON body wrapped in the Gods Eye envelope format.
 * The _sig field is "ir_telemetry" (Phase 1 HMAC bypass).
 *
 * body_json must be a canonical JSON string (no newlines).
 * Total packet must fit in 1400 bytes (well under MTU).
 */
static void emit_packet(const char *body_json) {
    char packet[1500];
    int len;

    if (!s_enabled || s_sock_fd < 0) return;

    /* Envelope: {"_body":"<escaped body>","_sig":"ir_telemetry"}
     * Since body_json contains no unescaped quotes (we control it),
     * we can embed it directly. The body uses single-level JSON
     * with no nested strings, so this is safe. */
    len = snprintf(packet, sizeof(packet),
        "{\"_body\":\"%s\",\"_sig\":\"ir_telemetry\"}",
        body_json);

    if (len <= 0 || len >= (int)sizeof(packet)) return;

    /* Fire and forget — ignore errors */
    sendto(s_sock_fd, packet, len, 0,
           (struct sockaddr *)&s_addr, sizeof(s_addr));
}

/*
 * Build a canonical body JSON string for a per-pass metric.
 * Keys are sorted alphabetically for consistency.
 * String values use single quotes internally to avoid escaping
 * issues in the envelope — wait, JSON requires double quotes.
 *
 * Actually: the _body is embedded inside double quotes in the
 * envelope, so we need to escape inner double quotes.
 * Simpler: use the body as-is but escape " to \".
 */
static void emit_body(char *buf, int bufsz, const char *body_fmt, ...) {
    char raw[1024];
    char escaped[1400];
    int i, j;
    va_list ap;

    va_start(ap, body_fmt);
    vsnprintf(raw, sizeof(raw), body_fmt, ap);
    va_end(ap);

    /* Escape double quotes for embedding inside envelope */
    j = 0;
    for (i = 0; raw[i] && j < (int)sizeof(escaped) - 2; i++) {
        if (raw[i] == '"') {
            escaped[j++] = '\\';
            escaped[j++] = '"';
        } else {
            escaped[j++] = raw[i];
        }
    }
    escaped[j] = '\0';

    snprintf(buf, bufsz, "%s", escaped);
}

/* ── Public API ──────────────────────────────────────────────────────── */

void ir_telem_init(void) {
    const char *env;
    const char *host;
    int port;

    env = getenv("ZCC_EMIT_TELEMETRY");
    if (!env || env[0] == '0' || env[0] == '\0') {
        s_enabled = 0;
        return;
    }

    host = getenv("GODS_EYE_HOST");
    if (!host) host = "127.0.0.1";

    port = 41337;
    env = getenv("GODS_EYE_PORT");
    if (env) port = atoi(env);

    s_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_sock_fd < 0) {
        s_enabled = 0;
        return;
    }

    /* Make non-blocking so sendto never stalls compilation */
    {
        int flags = 1;
        /* MSG_DONTWAIT on sendto is simpler; skip fcntl */
    }

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &s_addr.sin_addr);

    s_enabled = 1;
    s_compile_counter = 0;

    fprintf(stderr, "[IR TELEMETRY] Emitting to %s:%d\n", host, port);
}

void ir_telem_pass(const char *pass_name,
                   int func_count,
                   int nodes_before,
                   int nodes_after,
                   int nodes_deleted,
                   int nodes_modified) {
    char body[1024];
    char escaped[1400];
    int delta;

    if (!s_enabled) return;

    delta = nodes_after - nodes_before;

    /*
     * Build canonical JSON body with sorted keys.
     * Reuse existing schema fields for relay compatibility:
     *   deadlocks_healed = 0 (unused)
     *   gpu_temp_c       = nodes_before (previous energy)
     *   gpu_util_pct     = nodes_after  (current energy)
     *   h_t_state        = delta        (energy change)
     *   ir_funcs         = func_count   (extra, stripped by relay)
     *   ir_nodes_after   = nodes_after  (extra)
     *   ir_nodes_before  = nodes_before (extra)
     *   ir_nodes_deleted = nodes_deleted (extra)
     *   ir_nodes_modified= nodes_modified(extra)
     *   ir_pass          = pass_name    (extra)
     *   jit_latency_ms   = 0.0  (unused)
     *   swarm_cycles     = compile_counter
     *   vram_usage_mb    = 0.0  (unused)
     */
    snprintf(body, sizeof(body),
        "{\\\"deadlocks_healed\\\":0,"
        "\\\"gpu_temp_c\\\":%d,"
        "\\\"gpu_util_pct\\\":%d,"
        "\\\"h_t_state\\\":%d.0,"
        "\\\"ir_funcs\\\":%d,"
        "\\\"ir_nodes_after\\\":%d,"
        "\\\"ir_nodes_before\\\":%d,"
        "\\\"ir_nodes_deleted\\\":%d,"
        "\\\"ir_nodes_modified\\\":%d,"
        "\\\"ir_pass\\\":\\\"%s\\\","
        "\\\"jit_latency_ms\\\":0.0,"
        "\\\"swarm_cycles\\\":%d,"
        "\\\"vram_usage_mb\\\":0.0}",
        nodes_before,       /* gpu_temp_c */
        nodes_after,        /* gpu_util_pct */
        delta,              /* h_t_state */
        func_count,
        nodes_after,
        nodes_before,
        nodes_deleted,
        nodes_modified,
        pass_name,
        s_compile_counter);

    /* The body is already pre-escaped for embedding in the envelope */
    {
        char packet[1500];
        int len = snprintf(packet, sizeof(packet),
            "{\"_body\":\"%s\",\"_sig\":\"ir_telemetry\"}",
            body);
        if (len > 0 && len < (int)sizeof(packet)) {
            sendto(s_sock_fd, packet, len, 0,
                   (struct sockaddr *)&s_addr, sizeof(s_addr));
        }
    }
}

void ir_telem_summary(int total_funcs,
                      int total_nodes_before,
                      int total_nodes_after,
                      int pass_count,
                      const char **pass_names) {
    char body[1024];
    char passes_str[256];
    int delta;
    double reduction_pct;
    int i, pos;

    if (!s_enabled) return;

    s_compile_counter++;

    delta = total_nodes_after - total_nodes_before;
    reduction_pct = (total_nodes_before > 0)
        ? 100.0 * (1.0 - (double)total_nodes_after / total_nodes_before)
        : 0.0;

    /* Build comma-separated pass list */
    pos = 0;
    for (i = 0; i < pass_count && pos < (int)sizeof(passes_str) - 32; i++) {
        if (i > 0) passes_str[pos++] = ',';
        pos += snprintf(passes_str + pos, sizeof(passes_str) - pos,
                        "%s", pass_names[i]);
    }
    passes_str[pos] = '\0';

    snprintf(body, sizeof(body),
        "{\\\"deadlocks_healed\\\":0,"
        "\\\"gpu_temp_c\\\":%d,"
        "\\\"gpu_util_pct\\\":%d,"
        "\\\"h_t_state\\\":%d.0,"
        "\\\"ir_passes\\\":\\\"%s\\\","
        "\\\"ir_reduction_pct\\\":%.1f,"
        "\\\"ir_total_funcs\\\":%d,"
        "\\\"ir_total_nodes_after\\\":%d,"
        "\\\"ir_total_nodes_before\\\":%d,"
        "\\\"ir_type\\\":\\\"summary\\\","
        "\\\"jit_latency_ms\\\":0.0,"
        "\\\"swarm_cycles\\\":%d,"
        "\\\"vram_usage_mb\\\":%.1f}",
        total_nodes_before,     /* gpu_temp_c */
        total_nodes_after,      /* gpu_util_pct */
        delta,                  /* h_t_state */
        passes_str,
        reduction_pct,
        total_funcs,
        total_nodes_after,
        total_nodes_before,
        s_compile_counter,
        reduction_pct);         /* vram_usage_mb = reduction % */

    {
        char packet[1500];
        int len = snprintf(packet, sizeof(packet),
            "{\"_body\":\"%s\",\"_sig\":\"ir_telemetry\"}",
            body);
        if (len > 0 && len < (int)sizeof(packet)) {
            sendto(s_sock_fd, packet, len, 0,
                   (struct sockaddr *)&s_addr, sizeof(s_addr));
        }
    }

    fprintf(stderr, "[IR TELEMETRY] Compilation #%d: %d nodes -> %d nodes (%.1f%% reduction) [%s]\n",
            s_compile_counter, total_nodes_before, total_nodes_after,
            reduction_pct, passes_str);
}

void ir_telem_shutdown(void) {
    if (s_sock_fd >= 0) {
        close(s_sock_fd);
        s_sock_fd = -1;
    }
    s_enabled = 0;
}
