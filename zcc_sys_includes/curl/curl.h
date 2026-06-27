#ifndef ZCC_CURL_H
#define ZCC_CURL_H

typedef void CURL;
struct curl_slist {
    char *data;
    struct curl_slist *next;
};

typedef enum {
    CURLE_OK = 0
} CURLcode;

#define CURLOPT_WRITEDATA      10001
#define CURLOPT_URL            10002
#define CURLOPT_READDATA       10009
#define CURLOPT_WRITEFUNCTION  20011
#define CURLOPT_POSTFIELDS     10015
#define CURLOPT_HTTPHEADER     10023
#define CURLOPT_TIMEOUT        30013
#define CURLOPT_INFILESIZE     30115
#define CURLOPT_UPLOAD         10046

#define CURLINFO_RESPONSE_CODE 2097154

CURL *curl_easy_init(void);
CURLcode curl_easy_setopt(CURL *curl, int option, ...);
CURLcode curl_easy_perform(CURL *curl);
CURLcode curl_easy_getinfo(CURL *curl, int info, ...);
void curl_easy_cleanup(CURL *curl);
struct curl_slist *curl_slist_append(struct curl_slist *list, const char *str);
void curl_slist_free_all(struct curl_slist *list);
const char *curl_easy_strerror(CURLcode code);

#endif
