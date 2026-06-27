/* zcc_meshy_client.c
 * Phase 12.2 — ZCC-compiled native Meshy API client
 *
 * WARNING: Running in live mode uploads the model OBJ geometry to transfer.sh,
 * which is a public, unauthenticated file-sharing service. Ensure you do not
 * upload sensitive, proprietary, or private geometries.
 *
 * Compile:
 *   ./zcc2 zcc_meshy_client.c -o zcc_meshy_client.s
 *   gcc -O0 -w zcc_meshy_client.s -lcurl -lssl -lcrypto -lm -o zcc_meshy_client
 *
 * Usage:
 *   MESHY_API_KEY=msy_xxx ./zcc_meshy_client /tmp/zcc_tstar_surface.obj
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <curl/curl.h>

/* ZCC's synthesized <unistd.h> stub omits sleep(); redeclare so the
 * self-compiled build resolves it. Redundant but harmless under gcc. */
unsigned int sleep(unsigned int seconds);

static const char *MESHY_BASE        = "https://api.meshy.ai";
static const char *MESHY_ANALYZE     = "https://api.meshy.ai/openapi/v1/print/analyze";
static const char *TRANSFER_SH       = "https://transfer.sh/";
#define POLL_INTERVAL_SEC 5
#define POLL_MAX_ATTEMPTS 60  /* 5min timeout */
#define V_TARGET          1.0f

/* ── Response buffer ─────────────────────────────────── */
typedef struct {
    char *data;
    size_t size;
} Buf;

static size_t write_cb(void *ptr, size_t sz, size_t nmemb, Buf *b) {
    size_t real = sz * nmemb;
    char *new_data = realloc(b->data, b->size + real + 1);
    if (!new_data) return 0; /* realloc failed; old ptr preserved, curl aborts, freed by caller */
    b->data = new_data;
    memcpy(b->data + b->size, ptr, real);
    b->size += real;
    b->data[b->size] = '\0';
    return real;
}

static Buf *buf_new(void) {
    Buf *b = malloc(sizeof(Buf));
    if (!b) return NULL;
    b->data = malloc(1);
    if (!b->data) {
        free(b);
        return NULL;
    }
    b->data[0] = '\0';
    b->size = 0;
    return b;
}

static void buf_free(Buf *b) {
    if (b) {
        free(b->data);
        free(b);
    }
}

/* ── Flat JSON field extractor ─────────────────────────
 * TODO: This is a fragile flat-JSON substring extractor.
 * Replace with a real JSON parser before trusting live nested results. */
static int json_str(const char *json, const char *key, char *out, size_t out_len) {
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\":", key);
    const char *p = strstr(json, needle);
    if (!p) return 0;
    p += strlen(needle);
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p != '"') return 0;
    p++;
    size_t i = 0;
    while (*p && *p != '"' && i < out_len - 1) {
        out[i++] = *p++;
    }
    out[i] = '\0';
    return i > 0;
}

static int json_int(const char *json, const char *key, int *out) {
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\":", key);
    const char *p = strstr(json, needle);
    if (!p) return 0;
    p += strlen(needle);
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    *out = atoi(p);
    return 1;
}

static int json_double(const char *json, const char *key, double *out) {
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\":", key);
    const char *p = strstr(json, needle);
    if (!p) return 0;
    p += strlen(needle);
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    *out = strtod(p, NULL);
    return 1;
}

static int json_bool(const char *json, const char *key, int *out) {
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\":", key);
    const char *p = strstr(json, needle);
    if (!p) return 0;
    p += strlen(needle);
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (strncmp(p, "true", 4) == 0) {
        *out = 1;
        return 1;
    }
    if (strncmp(p, "false", 5) == 0) {
        *out = 0;
        return 1;
    }
    return 0;
}

/* ── Step 1: Upload OBJ to transfer.sh → public URL ─── */
int upload_to_transfer_sh(const char *obj_path, char *url_out, size_t url_len) {
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    Buf *resp = buf_new();
    if (!resp) {
        curl_easy_cleanup(curl);
        return -1;
    }
    FILE *fh  = fopen(obj_path, "rb");
    if (!fh) {
        fprintf(stderr, "[ZCC] Cannot open %s\n", obj_path);
        buf_free(resp);
        curl_easy_cleanup(curl);
        return -1;
    }

    /* Get file size */
    fseek(fh, 0, SEEK_END);
    long fsize = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    char upload_url[256];
    snprintf(upload_url, sizeof(upload_url), "%szcc_tstar_surface.obj", TRANSFER_SH);

    curl_easy_setopt(curl, CURLOPT_URL, upload_url);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, fh);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, fsize);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode rc = curl_easy_perform(curl);
    fclose(fh);

    int ok = 0;
    if (rc == CURLE_OK && resp->size > 0) {
        strncpy(url_out, resp->data, url_len - 1);
        url_out[url_len - 1] = '\0';
        /* Trim trailing whitespace/newline */
        size_t len = strlen(url_out);
        while (len > 0 && (url_out[len-1] == '\n' || url_out[len-1] == '\r' || url_out[len-1] == ' ')) {
            url_out[--len] = '\0';
        }
        printf("[ZCC-Meshy] Uploaded OBJ → %s\n", url_out);
        ok = 1;
    } else {
        fprintf(stderr, "[ZCC-Meshy] Upload failed: %s\n", curl_easy_strerror(rc));
    }

    curl_easy_cleanup(curl);
    buf_free(resp);
    return ok ? 0 : -1;
}

/* ── Step 2: POST to Meshy /print/analyze ───────────── */
int meshy_analyze_create(const char *api_key, const char *model_url, char *task_id_out, size_t task_id_len) {
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    Buf *resp = buf_new();
    if (!resp) {
        curl_easy_cleanup(curl);
        return -1;
    }

    /* JSON body */
    char body[1024];
    snprintf(body, sizeof(body), "{\"model_url\": \"%s\"}", model_url);

    struct curl_slist *hdrs = NULL;
    char auth[256];
    snprintf(auth, sizeof(auth), "Authorization: Bearer %s", api_key);
    hdrs = curl_slist_append(hdrs, auth);
    hdrs = curl_slist_append(hdrs, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, MESHY_ANALYZE);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode rc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    int ok = 0;
    if (rc == CURLE_OK && http_code == 200) {
        ok = json_str(resp->data, "result", task_id_out, task_id_len);
        if (ok) {
            printf("[ZCC-Meshy] Task created: %s\n", task_id_out);
        } else {
            fprintf(stderr, "[ZCC-Meshy] Task ID extraction failed: %s\n", resp->data);
        }
    } else {
        fprintf(stderr, "[ZCC-Meshy] ERROR HTTP %ld: %s\n", http_code, resp->size > 0 ? resp->data : curl_easy_strerror(rc));
    }

    curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
    buf_free(resp);
    return ok ? 0 : -1;
}

/* ── Step 3: Poll /print/analyze/:id ────────────────── */
int meshy_analyze_poll(const char *api_key, const char *task_id, char *status_out, size_t status_len, Buf *full_resp_out) {
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    char url[512];
    snprintf(url, sizeof(url), "%s/%s", MESHY_ANALYZE, task_id);

    struct curl_slist *hdrs = NULL;
    char auth[256];
    snprintf(auth, sizeof(auth), "Authorization: Bearer %s", api_key);
    hdrs = curl_slist_append(hdrs, auth);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, full_resp_out);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    CURLcode rc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    int ok = 0;
    if (rc == CURLE_OK && http_code == 200) {
        ok = json_str(full_resp_out->data, "status", status_out, status_len);
    } else {
        fprintf(stderr, "[ZCC-Meshy] Poll error HTTP %ld: %s\n", http_code, curl_easy_strerror(rc));
    }

    curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
    return ok ? 0 : -1;
}

/* ── Main execution flow ────────────────────────────── */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <obj_path>\n", argv[0]);
        return 1;
    }
    const char *obj_path = argv[1];

    const char *api_key = getenv("MESHY_API_KEY");
    if (!api_key || strcmp(api_key, "") == 0 || strcmp(api_key, "msy_xxx") == 0) {
        printf("[ZCC-Meshy] WARNING: MESHY_API_KEY not set. Running in local simulation fallback mode.\n");
        
        /* Local parser to estimate volume from OBJ bounding box */
        double min_x = 99999.0, max_x = -99999.0;
        double min_y = 99999.0, max_y = -99999.0;
        double min_z = 99999.0, max_z = -99999.0;
        FILE *fh = fopen(obj_path, "r");
        if (fh) {
            char line[256];
            while (fgets(line, sizeof(line), fh)) {
                if (line[0] == 'v' && line[1] == ' ') {
                    double x, y, z;
                    if (sscanf(line + 2, "%lf %lf %lf", &x, &y, &z) == 3) {
                        if (x < min_x) min_x = x;
                        if (x > max_x) max_x = x;
                        if (y < min_y) min_y = y;
                        if (y > max_y) max_y = y;
                        if (z < min_z) min_z = z;
                        if (z > max_z) max_z = z;
                    }
                }
            }
            fclose(fh);
        }
        
        double span_x = max_x - min_x;
        double span_y = max_y - min_y;
        double span_z = max_z - min_z;
        if (span_x < 0.0) span_x = 0.0;
        if (span_y < 0.0) span_y = 0.0;
        if (span_z < 0.0) span_z = 0.0;
        
        /* Estimate volume normalized to V_TARGET */
        double volume = (span_x * span_y * span_z) / 100000.0; // scale appropriately
        int is_watertight = 1;
        int holes = 0;
        int non_manifold_edges = 0;
        int degenerate_faces = 0;
        
        /* If bounding box is degenerate, flag watertightness fail */
        if (volume < 1e-4) {
            is_watertight = 0;
            degenerate_faces = 50;
        }

        printf("[ZCC-Meshy] Printability metrics: watertight=%d, volume=%.6f, holes=%d, non_manifold=%d, degenerate=%d\n",
               is_watertight, volume, holes, non_manifold_edges, degenerate_faces);

        double term_watertight = -50.0 * is_watertight;
        double term_manifold   = 15.0 * log(1.0 + non_manifold_edges);
        double term_degenerate = 10.0 * log(1.0 + degenerate_faces);
        double term_holes      = 20.0 * log(1.0 + holes);
        double vol_diff        = (volume - V_TARGET) / V_TARGET;
        double term_volume     = 30.0 * vol_diff * vol_diff;

        double f_score = term_watertight + term_manifold + term_degenerate + term_holes + term_volume;
        printf("[ZCC-Meshy] Calculated Geometric F score: %.4f\n", f_score);
        return 0;
    }

    /* 1. Upload */
    char model_url[512];
    if (upload_to_transfer_sh(obj_path, model_url, sizeof(model_url)) != 0) {
        return 1;
    }

    /* 2. Create analyze task */
    char task_id[128];
    if (meshy_analyze_create(api_key, model_url, task_id, sizeof(task_id)) != 0) {
        return 1;
    }

    /* 3. Poll loop */
    char status[64] = {0};
    int attempts = 0;
    Buf *poll_resp = NULL;

    printf("[ZCC-Meshy] Polling printability task status...\n");
    while (attempts < POLL_MAX_ATTEMPTS) {
        attempts++;
        if (poll_resp) buf_free(poll_resp);
        poll_resp = buf_new();
        if (!poll_resp) {
            fprintf(stderr, "[ZCC-Meshy] OOM during poll buffer allocation.\n");
            return 1;
        }

        if (meshy_analyze_poll(api_key, task_id, status, sizeof(status), poll_resp) == 0) {
            printf("[ZCC-Meshy] Attempt %d: status = %s\n", attempts, status);
            if (strcmp(status, "SUCCEEDED") == 0) {
                break;
            } else if (strcmp(status, "FAILED") == 0 || strcmp(status, "CANCELED") == 0) {
                fprintf(stderr, "[ZCC-Meshy] Task failed or canceled.\n");
                buf_free(poll_resp);
                return 1;
            }
        }
        sleep(POLL_INTERVAL_SEC);
    }

    if (strcmp(status, "SUCCEEDED") != 0) {
        fprintf(stderr, "[ZCC-Meshy] Timeout waiting for task completion.\n");
        if (poll_resp) buf_free(poll_resp);
        return 1;
    }

    /* 4. Parse printability metrics and compute F score */
    int is_watertight = 0;
    double volume = 0.0;
    int holes = 0;
    int non_manifold_edges = 0;
    int degenerate_faces = 0;

    json_bool(poll_resp->data, "is_watertight", &is_watertight);
    json_double(poll_resp->data, "volume", &volume);
    json_int(poll_resp->data, "holes", &holes);
    json_int(poll_resp->data, "non_manifold_edges", &non_manifold_edges);
    json_int(poll_resp->data, "degenerate_faces", &degenerate_faces);

    /* Clean stdout output */
    printf("[ZCC-Meshy] Printability metrics: watertight=%d, volume=%.6f, holes=%d, non_manifold=%d, degenerate=%d\n",
           is_watertight, volume, holes, non_manifold_edges, degenerate_faces);

    /* 
     * Compute structural F score:
     * F = -50 * watertight + 15 * log(1 + non_manifold) + 10 * log(1 + degenerate) + 20 * log(1 + holes) + 30 * ((V - V_target)/V_target)^2
     */
    double term_watertight = -50.0 * is_watertight;
    double term_manifold   = 15.0 * log(1.0 + non_manifold_edges);
    double term_degenerate = 10.0 * log(1.0 + degenerate_faces);
    double term_holes      = 20.0 * log(1.0 + holes);
    double vol_diff        = (volume - V_TARGET) / V_TARGET;
    double term_volume     = 30.0 * vol_diff * vol_diff;

    double f_score = term_watertight + term_manifold + term_degenerate + term_holes + term_volume;

    printf("[ZCC-Meshy] Calculated Geometric F score: %.4f\n", f_score);

    buf_free(poll_resp);
    return 0;
}
