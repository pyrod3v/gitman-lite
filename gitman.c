#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
char* get_config_dir();
char* read_file_content(const char* filepath);
void write_file_content(const char* filepath, const char* content);
char* fetch_gitignore(const char* type);
void handle_gitignore(const char* gitignore, const char* dir);

int main(int argc, char* argv[]) {
    char* dir = getcwd(NULL, 0);
    char* name = NULL;
    char* gitignore = NULL;
    char* user_name = NULL;
    char* user_email = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) {
            if (i + 1 < argc) {
                free(dir);
                dir = strdup(argv[++i]);
            } else {
                fprintf(stderr, "%s requires an argument. Using current directory.\n", argv[i]);
            }
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--name") == 0) {
            if (i + 1 < argc) {
                name = strdup(argv[++i]);
            } else {
                fprintf(stderr, "%s requires an argument. Skipping...\n", argv[i]);
            }
        } else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--gitignore") == 0) {
            if (i + 1 < argc) {
                gitignore = strdup(argv[++i]);
            } else {
                fprintf(stderr, "%s requires an argument. Skipping...\n", argv[i]);
            }
        } else if (strcmp(argv[i], "--user.name") == 0) {
            if (i + 1 < argc) {
                user_name = strdup(argv[++i]);
            } else {
                fprintf(stderr, "--user.name requires an argument. Skipping...\n");
            }
        } else if (strcmp(argv[i], "--user.email") == 0) {
            if (i + 1 < argc) {
                user_email = strdup(argv[++i]);
            } else {
                fprintf(stderr, "--user.email requires an argument. Skipping...\n");
            }
        }
    }

    char cmd[FILENAME_MAX];

    if (name) {
        char* dir_old = dir;
        size_t dir_len = strlen(dir) + strlen(name) + 2;
        char* new_dir = (char*)malloc(dir_len);
        if (!new_dir) {
            fprintf(stderr, "Memory allocation failed.\n");
            free(dir);
            return EXIT_FAILURE;
        }

        snprintf(new_dir, dir_len, "%s/%s", dir, name);

        free(dir);
        dir = new_dir;

        struct stat st = {0};
        if (stat(dir, &st) == -1) {
            if (mkdir(dir, 0700) == -1) {
                fprintf(stderr, "Failed to create directory %s: %s\n", dir, strerror(errno));
                free(dir);
                return EXIT_FAILURE;
            }
        }

        snprintf(cmd, sizeof(cmd), "git init %s", dir);
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to create repository at %s: %s\n", dir, strerror(errno));
        }
    }

    if (user_name) {
        snprintf(cmd, sizeof(cmd), "git -C %s config user.name \"%s\"", dir, user_name);
        int ret = system(cmd);
        if (ret == -1) {
            fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
        } else if (WIFEXITED(ret) && WEXITSTATUS(ret) != 0) {
            fprintf(stderr, "Failed to set git user.name (exit code: %d)\n", WEXITSTATUS(ret));
        }
    }

    if (user_email) {
        snprintf(cmd, sizeof(cmd), "git -C %s config user.email \"%s\"", dir, user_email);
        int ret = system(cmd);
        if (ret == -1) {
            fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
        } else if (WIFEXITED(ret) && WEXITSTATUS(ret) != 0) {
            fprintf(stderr, "Failed to set git user.name (exit code: %d)\n", WEXITSTATUS(ret));
        }
    }

    free(name);
    free(user_name);
    free(user_email);

    if (gitignore) 
        handle_gitignore(gitignore, dir);

    free(dir);
    free(gitignore);

    return EXIT_SUCCESS;
}

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
    const char* app_data = getenv("APPDATA");
    if (!app_data) {
        fprintf(stderr, "APPDATA environment variable not found.\n");
        return NULL;
    }

    size_t path_len = strlen(app_data) + strlen("\\gitman") + 1;
    config_dir = (char*)malloc(path_len);
    if (!config_dir) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    snprintf(config_dir, path_len, "%s\\gitman", app_data);
#else
    const char* home_dir = getenv("HOME");
    if (!home_dir) {
        fprintf(stderr, "HOME environment variable not found.\n");
        return NULL;
    }

    size_t path_len = strlen(home_dir) + strlen("/.config/gitman") + 1;
    config_dir = (char*)malloc(path_len);
    if (!config_dir) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    snprintf(config_dir, path_len, "%s/.config/gitman", home_dir);
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
