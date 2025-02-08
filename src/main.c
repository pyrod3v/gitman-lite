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
#include "gitman.h"

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

    if (name) {
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

        size_t cmd_len = strlen("git init ") + strlen(dir) + 1;
        char* cmd = (char*)malloc(cmd_len);
        if (!cmd) {
            fprintf(stderr, "Memory allocation failed.\n");
            free(dir);
            return EXIT_FAILURE;
        }

        snprintf(cmd, cmd_len, "git init %s", dir);
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to create repository at %s: %s\n", dir, strerror(errno));
        }
    }

    if (user_name) {
        size_t cmd_len = strlen("git -C ") + strlen(dir) + strlen(" config user.name \"\"") + strlen(user_name) + 1;
        char* cmd = (char*)malloc(cmd_len);
        if (!cmd) {
            fprintf(stderr, "Memory allocation failed.\n");
            free(dir);
            return EXIT_FAILURE;
        }

        snprintf(cmd, cmd_len, "git -C %s config user.name \"%s\"", dir, user_name);

        int ret = system(cmd);
        if (ret == -1) {
            fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
        } else if (WIFEXITED(ret) && WEXITSTATUS(ret) != 0) {
            fprintf(stderr, "Failed to set git user.name (exit code: %d)\n", WEXITSTATUS(ret));
        }
    }

    if (user_email) {
        size_t cmd_len = strlen("git -C ") + strlen(dir) + strlen(" config user.email \"\"") + strlen(user_email) + 1;
        char* cmd = (char*)malloc(cmd_len);
        if (!cmd) {
            fprintf(stderr, "Memory allocation failed.\n");
            free(dir);
            return EXIT_FAILURE;
        }

        snprintf(cmd, cmd_len, "git -C %s config user.email \"%s\"", dir, user_email);

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
