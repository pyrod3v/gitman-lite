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

#ifndef GITMAN_H

#include <unistd.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
char* get_config_dir();
char* read_file_content(const char* filepath);
void write_file_content(const char* filepath, const char* content);
char* fetch_gitignore(const char* type);
void handle_gitignore(const char* gitignore, const char* dir);

#endif