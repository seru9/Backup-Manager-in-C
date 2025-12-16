#include "file_ops.h"
#include "signals.h"


ssize_t bulk_write(int fd, char *buf, size_t count) {
    ssize_t c;
    ssize_t len = 0;
    do {
        c = TEMP_FAILURE_RETRY(write(fd, buf + len, count - len));
        if (c < 0) return c;
        len += c;
    } while ((size_t)len < count); // wiemy, że len > 0
    return len;
}

ssize_t bulk_read(int fd, char *buf, size_t count) {
    ssize_t c;
    ssize_t len = 0;
    do {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0) return c;
        if (c == 0) return len; // EOF
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

void copy_reg_file(const char *src_path, const char *dst_path) {
    if (should_stop()) return;

    int src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) {
        perror("Błąd otwarcia pliku źródłowego");
        return;
    }

    struct stat st;
    fstat(src_fd, &st);
    int dst_fd = TEMP_FAILURE_RETRY(open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode));
    if (dst_fd < 0) {
        perror("Błąd utworzenia pliku docelowego");
        close(src_fd);
        return;
    }

    char buf[32];
    ssize_t bytes_read;
    while ((bytes_read = bulk_read(src_fd, buf, sizeof(buf))) > 0) {
        if (should_stop()) break;
        if (bulk_write(dst_fd, buf, bytes_read) < 0) {
            ERR("write");
            break;
        }
    }

    TEMP_FAILURE_RETRY(close(src_fd));
    TEMP_FAILURE_RETRY(close(dst_fd));
}

void handle_symlink(char *src_path, char *dst_path, char *root_src, char *root_dst) {
    if (should_stop()) return;

    char link_target[PATH_MAX];
    ssize_t len = readlink(src_path, link_target, sizeof(link_target) - 1);
    if (len == -1) {
        perror("readlink");
        return;
    }
    link_target[len] = '\0';

    char new_target[PATH_MAX];
    size_t root_src_len = strlen(root_src);
    if (link_target[0] == '/' && strncmp(link_target, root_src, root_src_len) == 0) {
        snprintf(new_target, sizeof(new_target), "%s%s", root_dst, link_target + root_src_len);
    } else { // Kleimy linka, albo w tym samym folderze na górze, albo linker jest na folder z zewnatrz
        strncpy(new_target, link_target, sizeof(new_target));
    }

    unlink(dst_path);
    if (symlink(new_target, dst_path) == -1) {
        perror("symlink creation failed");
    }
}

void recursive_copy_worker(char *current_src, char *current_dst, char *root_src, char *root_dst) {
    if (should_stop()) return;

    DIR *dir = opendir(current_src);
    if (!dir) return;

    struct stat st;
    if (stat(current_src, &st) == 0) { // do poprawy
        mkdir(current_dst, st.st_mode);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (should_stop()) break;

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char next_src[PATH_MAX];
        char next_dst[PATH_MAX];

        snprintf(next_src, sizeof(next_src), "%s/%s", current_src, entry->d_name);
        snprintf(next_dst, sizeof(next_dst), "%s/%s", current_dst, entry->d_name);

        struct stat entry_stat;
        if (lstat(next_src, &entry_stat) == -1) continue;

        if (S_ISDIR(entry_stat.st_mode)) {
            recursive_copy_worker(next_src, next_dst, root_src, root_dst);
        } else if (S_ISLNK(entry_stat.st_mode)) {
            handle_symlink(next_src, next_dst, root_src, root_dst);
        } else if (S_ISREG(entry_stat.st_mode)) {
            copy_reg_file(next_src, next_dst);
        }

    }
    closedir(dir);
}

void make_copy(char source_path[], char target_path[]) {
    if (should_stop()) return;
    recursive_copy_worker(source_path, target_path, source_path, target_path);
}
int check_path(char* input_path, int is_source_path) {
    DIR *dir = opendir(input_path);
    if (is_source_path == 1) {
        if (dir) {
            closedir(dir); 
            return 1;      
        }
        return 0;          
    }

    if (!dir) {
        createDir(input_path);
        
        char temp_abs[MAX_PATH_LEN];
        resolve_absolute_path(input_path, temp_abs);
        
        strncpy(input_path, temp_abs, MAX_PATH_LEN - 1);
        input_path[MAX_PATH_LEN - 1] = '\0';

        return 1; 
    } 
    else {
        struct dirent *entry;
        int is_empty = 1; 

        while ((entry = readdir(dir)) != NULL) {
            // Ignorujemy "." i ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // Znaleziono coś innego -> katalog NIE jest pusty
            is_empty = 0;
            break;
        }
        
        closedir(dir); 
        return is_empty; 
    }
}
int check_source_target(const char* source_path, const char* target_path, pid_source_target pid_archived[], int pid_archived_size){ //wymagania zadania
    size_t src_len = strlen(source_path);
    if (strncmp(target_path, source_path, src_len) == 0) {
        return 0;
    }
    for(int i = 0; i < pid_archived_size; i++){
        if(strcmp(source_path, pid_archived[i].source_path) == 0 && strcmp(target_path, pid_archived[i].target_path) == 0){
            return 0;
        }
    }
    return 1;

}
void createDir(const char *path) {
    char temp[1024];
    char *p = NULL;
    size_t len;

    // Kopiujemy ścieżkę do zmiennej tymczasowej, bo będziemy ją modyfikować
    snprintf(temp, sizeof(temp), "%s", path);
    len = strlen(temp);

    if (temp[len - 1] == '/' || temp[len - 1] == '\\') {
        temp[len - 1] = 0;
    }

    for (p = temp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char save = *p; 
            *p = 0;         

            mkdir(temp, 0777); 
            
            *p = save;     
        }
    }
    
    mkdir(temp, 0777);
}
