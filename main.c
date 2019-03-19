#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


int do_ls(char*, int);
int is_a_option(int);
int is_l_option(int);
void full_display(const struct stat, const char*);


int main(int argc, char* argv[]) {
    int mode = 0;

    // Parse options, set mode.
    char temp = 0;
    while((temp = getopt(argc, argv, "al")) != -1) {
        switch(temp) {
            case 'a':
                mode |= 0x1;
                break;
            case 'l':
                mode |= 0x2;
                break;
        }
    }

    return do_ls(argv[optind], mode);
}


int is_a_option(int mode) {
    return mode & 0x1;
}


int is_l_option(int mode) {
    return mode & 0x2;
}


int do_ls(char* path, int mode) {
    // Check path argument is given.
    if(path) {
        struct stat stat;
        if(lstat(path, &stat) == -1) {
            fprintf(stderr, "Failed to read file stat: %s: %s\n", strerror(errno), path);
            return -1;
        }
        if(!S_ISDIR(stat.st_mode)) {
            int i = 0;
            for(i = strlen(path); i >= 0; i--) {
                if(path[i] == '/') break;
            }
            i++;
            if(is_l_option(mode)) full_display(stat, &(path[i]));
            else printf("%s\n", &(path[i]));
            return 0;
        }
    } else {
        path = (char*) malloc(1024);
        if(getcwd(path, 1024) == NULL) {
            fprintf(stderr, "Filaed to get current working directory: %s\n", strerror(errno));
            return -1;
        }
    }

    DIR* dir = opendir(path);
    if(!dir) {
        fprintf(stderr, "Failed to open such directory: %s: %s\n", strerror(errno), path);
        return -1;
    }

    struct dirent* entry;
    while(entry = readdir(dir)) {
        if(is_l_option(mode)) {
            // Get full path to get file stat.
            char* full_path = (char*) malloc(strlen(path) + entry->d_reclen + 1);
            strcpy(full_path, path);
            strcat(full_path, "/");
            strcat(full_path, &(entry->d_name[0]));

            // Get stat.
            struct stat file_stat;
            if(lstat(full_path, &file_stat) == -1) {
                fprintf(stderr, "Failed to read file stat: %s: %s\n", strerror(errno), full_path);
                continue;
            }
            
            // Display
            if(entry->d_name[0] == '.') {
                if(is_a_option(mode)) full_display(file_stat, entry->d_name);
            } else {
                full_display(file_stat, entry->d_name);
            }

            free(full_path);
        } else {
            // Display
            if(entry->d_name[0] == '.') {
                if(is_a_option(mode)) printf("%s  ", entry->d_name);
            } else {
                printf("%s  ", entry->d_name);
            }
        }
    }
    if(!is_l_option(mode)) printf("\n");

    closedir(dir);
    return 0;
}


void full_display(const struct stat stat, const char* filename) {
    // Print permission.
    printf("%c%c%c%c%c%c%c%c%c%c ",
            S_ISDIR(stat.st_mode) ? 'd' : '-',
            stat.st_mode & S_IRUSR ? 'r' : '-',
            stat.st_mode & S_IWUSR ? 'w' : '-',
            stat.st_mode & S_IXUSR ? 'x' : '-',
            stat.st_mode & S_IRGRP ? 'r' : '-',
            stat.st_mode & S_IWGRP ? 'w' : '-',
            stat.st_mode & S_IXGRP ? 'x' : '-',
            stat.st_mode & S_IROTH ? 'r' : '-',
            stat.st_mode & S_IWOTH ? 'w' : '-',
            stat.st_mode & S_IXOTH ? 'x' : '-');

    printf("%ld\t", stat.st_nlink);

    // Print owner.
    struct passwd* user = getpwuid(stat.st_uid);
    printf("%s ", user->pw_name);

    // Print owner group.
    struct group* group = getgrgid(stat.st_gid);
    printf("%s ", group->gr_name);

    printf("%ld\t", stat.st_size);

    // Print last modified time.
    struct tm* modified_time = localtime(&(stat.st_mtime));
    switch(modified_time->tm_mon) {
        case 0:
            printf("Jan ");
            break;
        case 1:
            printf("Feb ");
            break;
        case 2:
            printf("Mar ");
            break;
        case 3:
            printf("Apr ");
            break;
        case 4:
            printf("May ");
            break;
        case 5:
            printf("Jun ");
            break;
        case 6:
            printf("Jul ");
            break;
        case 7:
            printf("Aug ");
            break;
        case 8:
            printf("Sep ");
            break;
        case 9:
            printf("Oct ");
            break;
        case 10:
            printf("Nov ");
            break;
        case 11:
            printf("Dec ");
            break;
    }
    if(time(NULL) - 15780000 < stat.st_mtime)
        printf("%2d %.2d:%.2d ", modified_time->tm_mday, modified_time->tm_hour, modified_time->tm_min);
    else
        printf("%2d  %d ", modified_time->tm_mday, modified_time->tm_year+1900);

    printf("%s%c\n", filename, S_ISDIR(stat.st_mode) ? '/' : '\0');
}
