// Copyright 2025 pyrod3v
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <errno.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    char** buffer = (char**)userp;
    size_t oldSize = *buffer ? strlen(*buffer) : 0;

    char* temp = (char*)realloc(*buffer, oldSize + totalSize + 1);
    if (!temp) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 0;
    }

    *buffer = temp;
    memcpy(*buffer + oldSize, contents, totalSize);
    (*buffer)[oldSize + totalSize] = '\0';
    return totalSize;
}

char* fetch_gitignore(const char* type) {
    CURL* curl;
    CURLcode res;
    char* buffer = (char*)malloc(1);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }
    buffer[0] = '\0';

    curl = curl_easy_init();
    if (curl) {
        size_t url_size = strlen("https://www.toptal.com/developers/gitignore/api/") + strlen(type) + 1;
        char* url = (char*)malloc(url_size);
        if (!url) {
            fprintf(stderr, "Memory allocation failed.\n");
            free(buffer);
            return NULL;
        }

        snprintf(url, url_size, "https://www.toptal.com/developers/gitignore/api/%s", type);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        free(url);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(buffer);
            return NULL;
        }
    }
    return buffer;
}

char* get_config_dir() {
    char* config_dir = NULL;
#ifdef _WIN32
    const char* home = getenv("HOMEPATH");
    if (!home) {
        fprintf(stderr, "HOMEPATH environment variable not found.\n");
        return NULL;
    }

    size_t path_len = strlen(home) + strlen("\\.gitman") + 1;
    config_dir = (char*)malloc(path_len);
    if (!config_dir) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    snprintf(config_dir, path_len, "%s\\.gitman", home);
#else
    const char* home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "HOME environment variable not found.\n");
        return NULL;
    }

    size_t path_len = strlen(home) + strlen("/.gitman") + 1;
    config_dir = (char*)malloc(path_len);
    if (!config_dir) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    snprintf(config_dir, path_len, "%s/.gitman", home);
#endif

    struct stat st = {0};
    if (stat(config_dir, &st) == -1) {
        if (mkdir(config_dir, 0700) == -1) {
            fprintf(stderr, "Failed to create directory %s: %s\n", config_dir, strerror(errno));
            free(config_dir);
            return NULL;
        }
    }

    return config_dir;
}

char* read_file_content(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char* content = malloc(length + 1);
    if (!content) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    fread(content, 1, length, file);
    content[length] = '\0';
    fclose(file);

    return content;
}

void write_file_content(const char* filepath, const char* content) {
    FILE* file = fopen(filepath, "w");
    if (!file) {
        fprintf(stderr, "Failed to open file for writing: %s\n", filepath);
        return;
    }

    fputs(content, file);
    fclose(file);
}

void handle_gitignore(const char* gitignore, const char* dir) {
    char* config_dir = get_config_dir();
    char gitignore_path[FILENAME_MAX];
    char* gitignore_content = NULL;

    if (config_dir) {
        snprintf(gitignore_path, sizeof(gitignore_path), "%s/gitignores/%s.gitignore", config_dir, gitignore);
        gitignore_content = read_file_content(gitignore_path);
    }

    if (!gitignore_content) {
        fprintf(stderr, "Fetching .gitignore for %s...\n", gitignore);
        gitignore_content = fetch_gitignore(gitignore);

        if (gitignore_content && config_dir) {
            snprintf(gitignore_path, sizeof(gitignore_path), "%s/gitignores", config_dir);
            struct stat st = {0};
            if (stat(gitignore_path, &st) == -1) {
                if (mkdir(gitignore_path, 0700) == -1) {
                    fprintf(stderr, "Failed to create directory %s: %s\n", gitignore_path, strerror(errno));
                    free(config_dir);
                    return;
                }
            }

            snprintf(gitignore_path, sizeof(gitignore_path), "%s/gitignores/%s", config_dir, gitignore);
            write_file_content(gitignore_path, gitignore_content);
        }
    }

    free(config_dir);

    if (gitignore_content) {
        char final_gitignore_path[FILENAME_MAX];
        snprintf(final_gitignore_path, sizeof(final_gitignore_path), "%s/.gitignore", dir);
        write_file_content(final_gitignore_path, gitignore_content);
        free(gitignore_content);
    } else {
        fprintf(stderr, "Failed to obtain .gitignore for %s.\n", gitignore);
    }
}
