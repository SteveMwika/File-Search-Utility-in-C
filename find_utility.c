#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

// Function prototypes
void find_files(char *dir_path, char *criteria, char *action, int argc, char *argv[]);
void find_and_print(char *dir_path);
void find_by_name(char *dir_path, char *name);
void find_by_mmin(char *dir_path, int mmin);
void find_by_inum(char *dir_path, ino_t inum);
void delete_files(char *dir_path, char *criteria, int argc, char *argv[]);
void execute_command(char *dir_path, char *criteria, char *command);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <directory> <criteria> [action]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *dir_path = argv[1];
    char *criteria = argv[2];
    char *action = (argc > 3) ? argv[3] : NULL;

    find_files(dir_path, criteria, action, argc, argv);

    return 0;
}

void delete_files(char *dir_path, char *criteria, int argc, char *argv[]) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
            char file_path[PATH_MAX];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);

            if ((entry->d_type == DT_REG && strcmp(entry->d_name, criteria) == 0) ||
                (entry->d_type == DT_DIR && strstr(entry->d_name, criteria) != NULL)) {
                if (remove(file_path) == 0) {
                    printf("Deleted: %s\n", file_path);
                } else {
                    perror("remove");
                }
            }

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                // Recursive call for subdirectories
                delete_files(file_path, criteria, argc, argv);
            }
        }
    }

    closedir(dir);
}

void find_files(char *dir_path, char *criteria, char *action, int argc, char *argv[]) {
    if (strcmp(criteria, "where-to-look") == 0) {
        find_and_print(dir_path);
    } else if (strcmp(criteria, "-name") == 0) {
        if (argc > 3 && strcmp(argv[3], "-exec") == 0) {
            if (argc > 4) {
                find_by_name(dir_path, action);
                execute_command(dir_path, action, argv[4]);
            } else {
                printf("Missing command for -exec\n");
                exit(EXIT_FAILURE);
            }
        } else {
            find_by_name(dir_path, action);
        }
    } else if (strcmp(criteria, "-mmin") == 0) {
        int mmin = atoi(action);
        find_by_mmin(dir_path, mmin);
    } else if (strcmp(criteria, "-inum") == 0) {
        ino_t inum = (ino_t)atoi(action);
        find_by_inum(dir_path, inum);
    } else if (strcmp(criteria, "-delete") == 0) {
        delete_files(dir_path, action, argc, argv);
    } else if (strcmp(criteria, "-exec") == 0) {
        if (argc > 4) {
            execute_command(dir_path, action, argv[4]);
        } else {
            printf("Missing command for -exec\n");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Invalid criteria: %s\n", criteria);
        exit(EXIT_FAILURE);
    }
}



void execute_command(char *dir_path, char *criteria, char *command) {
    // Implement code to execute a command
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
            char file_path[PATH_MAX];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);

            if (strcmp(entry->d_name, criteria) == 0) {
                char cmd[PATH_MAX + 20];  // Assuming the command won't be too long
                snprintf(cmd, sizeof(cmd), "%s %s", command, file_path);
                if (system(cmd) == 0) {
                    printf("Executed: %s\n", cmd);
                } else {
                    perror("system");
                }
            }

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                // Recursive call for subdirectories
                execute_command(file_path, criteria, command);
            }
        }
    }

    closedir(dir);
}

void find_and_print(char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
            char file_path[PATH_MAX];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);

            printf("%s\n", file_path);

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                // Recursive call for subdirectories
                find_and_print(file_path);
            }
        }
    }

    closedir(dir);
}

void find_by_name(char *dir_path, char *name) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
            char file_path[PATH_MAX];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);

            if (strcmp(entry->d_name, name) == 0) {
                printf("%s\n", file_path);
            }

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                // Recursive call for subdirectories
                find_by_name(file_path, name);
            }
        }
    }

    closedir(dir);
}

void find_by_mmin(char *dir_path, int mmin) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
            char file_path[PATH_MAX];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);

            struct stat file_stat;
            if (stat(file_path, &file_stat) == -1) {
                perror("stat");
                exit(EXIT_FAILURE);
            }

            int minutes_ago = (int)(difftime(time(NULL), file_stat.st_mtime) / 60);

            if (mmin < 0 && minutes_ago < abs(mmin)) {
                printf("%s\n", file_path);
            } else if (mmin == 0 && minutes_ago == 0) {
                printf("%s\n", file_path);
            } else if (mmin > 0 && minutes_ago > mmin) {
                printf("%s\n", file_path);
            }

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                // Recursive call for subdirectories
                find_by_mmin(file_path, mmin);
            }
        }
    }

    closedir(dir);
}

void find_by_inum(char *dir_path, ino_t inum) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
            char file_path[PATH_MAX];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);

            struct stat file_stat;
            if (stat(file_path, &file_stat) == -1) {
                perror("stat");
                exit(EXIT_FAILURE);
            }

            if (file_stat.st_ino == inum) {
                printf("%s\n", file_path);
            }

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                // Recursive call for subdirectories
                find_by_inum(file_path, inum);
            }
        }
    }

    closedir(dir);
}
