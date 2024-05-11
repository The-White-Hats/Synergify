#include "header.h"

/**
 * getAbsolutePath - takes a file_name and appends it to an absolute_path.
 *
 * @absolute_path: pointer to the buffer where the absolute path will be stored.
 * @file_name: name of the file to append.
 */
void getAbsolutePath(char *const absolute_path, const char *const file_name) {
    char executable_path[PATH_SIZE];

    // Get the path to the executable
    ssize_t len = readlink("/proc/self/exe", executable_path, sizeof(executable_path) - 1);
    if (len == -1) {
        perror("readlink");
        exit(EXIT_FAILURE);
    }
    executable_path[len] = '\0'; // Null-terminate the string

    // Find the last occurrence of '/' to get the directory path
    char *last_slash = strrchr(executable_path, '/');
    if (last_slash == NULL) {
        fprintf(stderr, "Error: Unable to extract directory path from executable path\n");
        exit(EXIT_FAILURE);
    }
    *(last_slash + 1) = '\0'; // Null-terminate at the last slash to get the directory path

    strcpy(absolute_path, executable_path);
    strcat(absolute_path, file_name);
}

/**
 * getProjectPath - takes a file_name and appends it to the project path.
 *
 * @absolute_path: pointer to the buffer where the absolute path will be stored.
 * @file_name: name of the file to append.
 */
void getProjectPath(char *const absolute_path, const char *const file_name) {
    char executable_path[PATH_SIZE];

    // Get the path to the executable
    ssize_t len = readlink("/proc/self/exe", executable_path, sizeof(executable_path) - 1);
    if (len == -1) {
        perror("readlink");
        exit(EXIT_FAILURE);
    }
    executable_path[len] = '\0'; // Null-terminate the string

    // Find the last occurrence of '/' to get the directory path
    char *last_slash = strrchr(executable_path, '/');
    if (last_slash == NULL) {
        fprintf(stderr, "Error: Unable to extract directory path from executable path\n");
        exit(EXIT_FAILURE);
    }
    *last_slash = '\0'; // Null-terminate at the last slash to get the directory path

    // Find the second last occurrence of '/' to get the parent directory
    last_slash = strrchr(executable_path, '/');
    if (last_slash == NULL) {
        fprintf(stderr, "Error: Unable to extract parent directory path from executable path\n");
        exit(EXIT_FAILURE);
    }
    *(last_slash + 1) = '\0'; // Null-terminate at the second last slash to get the parent directory

    strcpy(absolute_path, executable_path);
    strcat(absolute_path, file_name);
}