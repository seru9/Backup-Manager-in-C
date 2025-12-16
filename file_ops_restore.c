#include "file_ops_restore.h"

int remove_recursive(char *path) {
    struct stat st;
    if (lstat(path, &st) < 0) return -1; // od tego mam handle_symlink dlatego -1

    if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(path);
        if (!dir) return -1;
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char sub_path[PATH_MAX];
            snprintf(sub_path, sizeof(sub_path), "%s/%s", path, entry->d_name);
            remove_recursive(sub_path);
        }
        closedir(dir);
        return rmdir(path); // usuwam całe directory
    } else {
        return unlink(path); //usuwam poprostu plik
    }
}

//Usuwanie plików ze źródła, których nie ma w backupie
void delete_extras(char *source_root, char *target_root) {
    DIR *dir = opendir(source_root);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char src_abs[PATH_MAX]; // Ścieżka absolutna w źródle
        char tgt_abs[PATH_MAX]; // Odpowiadająca jej ścieżka w backupie

        snprintf(src_abs, sizeof(src_abs), "%s/%s", source_root, entry->d_name);
        snprintf(tgt_abs, sizeof(tgt_abs), "%s/%s", target_root, entry->d_name);

        struct stat st;
        // Jeśli plik nie istnieje w backupie -> usuń go ze źródła
        //ENOENT no entry
        if (lstat(tgt_abs, &st) == -1 && errno == ENOENT) {
            remove_recursive(src_abs);
        } else if (S_ISDIR(st.st_mode)) {
            delete_extras(src_abs, tgt_abs);
        }
        // Jeśli istnieje, ale typ się nie zgadza plik i katalog,

    }
    closedir(dir);
}


void recursive_restore(char *curr_source, char *curr_backup, char *source_root, char *backup_root) {
    DIR *dir = opendir(curr_backup);
    if (!dir) return;

    struct stat st_dir;
    if (stat(curr_backup, &st_dir) == 0) {
        mkdir(curr_source, st_dir.st_mode); // Upewnij się, że katalog istnieje
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char next_src[PATH_MAX];
        char next_bkp[PATH_MAX];

        snprintf(next_src, sizeof(next_src), "%s/%s", curr_source, entry->d_name);
        snprintf(next_bkp, sizeof(next_bkp), "%s/%s", curr_backup, entry->d_name);
        // Dla każdego entry z curr__backup robie dwa nowe ścieżki nex_bkp i next_src
        struct stat bkp_stat, src_stat;
        if (lstat(next_bkp, &bkp_stat) == -1) continue;

        // Decyzja czy kopiować
        int need_copy = 1;
        if (lstat(next_src, &src_stat) == 0) {
            // Jeśli to zwykły plik, sprawdź datę modyfikacji i rozmiar, czy je taka sama, wówczas 
            // nie jest potrzebna kopia
            if (S_ISREG(bkp_stat.st_mode) && S_ISREG(src_stat.st_mode)) {
                if (bkp_stat.st_mtime == src_stat.st_mtime && bkp_stat.st_size == src_stat.st_size) {
                    need_copy = 0;
                }
            }
            // Jeśli typ pliku jest inny (np. był katalog, jest plik), trzeba nadpisać
            if ((bkp_stat.st_mode & S_IFMT) != (src_stat.st_mode & S_IFMT)) {
                remove_recursive(next_src); // Usuń stary typ
                need_copy = 1;
            }
        }

        if (S_ISDIR(bkp_stat.st_mode)) {
            recursive_restore(next_src, next_bkp, source_root, backup_root);
        } else if (need_copy) {
            if (S_ISLNK(bkp_stat.st_mode)) {
                handle_symlink(next_bkp, next_src, backup_root, source_root);
            } else if (S_ISREG(bkp_stat.st_mode)) {
                copy_reg_file(next_bkp, next_src); // Kopiowanie z Backupu -> Źródła
                // Przywracanie czasu modyfikacji (ważne dla kolejnych sprawdzeń)
                struct timespec times[2];
                times[0] = bkp_stat.st_atim;
                times[1] = bkp_stat.st_mtim;
                utimensat(AT_FDCWD, next_src, times, AT_SYMLINK_NOFOLLOW); // funkcja, na przypisanie next_src takiego samego 
                //at_symlink_nofollow przypisujemy samemu linkowi
            }
        }
    }
    closedir(dir);
}

void perform_restore(char *source_path, char *target_path) {
    // source_path -> Tam gdzie przywracamy (np. /home/user/praca)
    // target_path -> Skąd bierzemy (Backup) (np. /tmp/backup_pracy)
    
    // Wyczyść źródło z plików, których nie ma w backupie
    delete_extras(source_path, target_path);

    //Przywróć pliki i katalogi (tylko zmienione) 
    recursive_restore(source_path, target_path, source_path, target_path);
}