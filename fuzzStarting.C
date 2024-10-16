#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>

#define MAX_FILE_SIZE 1024

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        // Out of memory
        fprintf(stderr, "Not enough memory (realloc returned NULL)
");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

void fuzzme(char *buf, size_t size) {
    // Basic example of fuzzing logic: flip random bits in the buffer
    for (size_t i = 0; i < size; ++i) {
        buf[i] ^= rand() % 256;
    }
    // Replace this with specific fuzz testing you need
    // e.g., searching for specific patterns, modifying inputs, etc.
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s URL
", argv[0]);
        return 1;
    }

    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;
    // will be grown as needed by the realloc
    chunk.memory = malloc(1);  
    // no data at this point
    chunk.size = 0;            

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, argv[1]);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl_handle);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s
", curl_easy_strerror(res));
        return 1;
    }

    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    if(chunk.size > MAX_FILE_SIZE) {
        fprintf(stderr, "Downloaded content exceeds maximum allowed size of %d bytes
", MAX_FILE_SIZE);
        return 1;
    }

    srand((unsigned int)time(NULL));
    fuzzme(chunk.memory, chunk.size);

    free(chunk.memory);

    return 0;
}