#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int match(char *path, char *target)
{
    int lpath, ltarget;
    lpath = strlen(path);

    ltarget = strlen(target);
    if (lpath < ltarget) {
        return 1;
    }

    // as strcmp(strrchar(path, '/'), target)
    return strcmp(path + (lpath - ltarget), target);
}

void find(char *path, char *str)
{
    int fd;
    struct stat file_stat;
    struct dirent dir_info;
    char path_buf[512], *child_path;

    if ((fd = open(path, 0)) < 0) {
      fprintf(2, "find: cannot open %s\n", path);
      return;
    }

    if (fstat(fd, &file_stat) < 0) {
      fprintf(2, "find: cannot stat %s\n", path);
      close(fd);
      return;
    }

    switch(file_stat.type) {
        case T_FILE:
            if (match(path, str) == 0) {
                printf("%s\n", path);
            }
            break;

        case T_DIR:
            strcpy(path_buf, path);
            child_path = path_buf + strlen(path_buf);
            *child_path++ = '/'; // replace '\0' to '/'
            while (read(fd, &dir_info, sizeof(dir_info)) == sizeof(dir_info)) {
                if (dir_info.inum == 0 ||
                  strcmp(dir_info.name, ".") == 0 ||
                  strcmp(dir_info.name, "..") == 0) {
                    continue;
                }
                memmove(child_path, dir_info.name, DIRSIZ);  // as strcat(child_path, name)
                child_path[DIRSIZ] = '\0'; // reset child_path for next read
                find(path_buf, str); // recurse call child_path
            }
            break;

        default: // ignore type device
            break;
    }
  
    close(fd);
}

int main(int argc, char *argv[])
{
    find(argv[1], argv[2]);
    exit(0);
}