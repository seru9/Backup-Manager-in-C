#include "inotify.h"

WatchMap watches[MAX_WATCHERS]; 
int watches_count = 0;

int add_watch_to_map(int wd, const char *path) {
    if (watches_count >= MAX_WATCHERS) {
        fprintf(stderr, "[WARN] Limit obserwowanych katalogow (%d) osiagniety! Nie dodano: %s\n", MAX_WATCHERS, path);
        return -1;
    }
    
    watches[watches_count].wd = wd;
    
    // POPRAWKA 1 i 2: Bezpieczne kopiowanie i gwarancja zera na końcu
    strncpy(watches[watches_count].path, path, MAX_PATH_LEN - 1);
    watches[watches_count].path[MAX_PATH_LEN - 1] = '\0'; // Zawsze kończ zerem
    
    watches_count++;
    return 0;
}

char* get_path_by_wd(int wd) {
    for (int i = 0; i < watches_count; i++) {
        if (watches[i].wd == wd) return watches[i].path;
    }
    return NULL;
}
// Funkcja znajdująca ścieżkę na podstawie numeru WD
void add_watch_recursive(int fd, const char *root_path) {
    // POPRAWKA 3: Najpierw sprawdź, czy mamy miejsce w tablicy!
    if (watches_count >= MAX_WATCHERS) {
        fprintf(stderr, "[WARN] Limit osiągnięty przed dodaniem: %s\n", root_path);
        return;
    }

    // Dodanie watchera do inotify
    // Zauważyłem, że dodałeś IN_CLOSE_WRITE - to dobry pomysł przy kopiowaniu plików!
    int wd = inotify_add_watch(fd, root_path,
                               IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM |
                               IN_MOVED_TO | IN_CLOSE_WRITE | IN_DONT_FOLLOW); // IN_DONT_FOLLOW warto dodać dla bezpieczeństwa
   
    if (wd < 0) {
        // Jeśli katalog nie istnieje lub brak uprawnień - ignorujemy i nie wchodzimy głębiej
        if (errno != EACCES && errno != ENOENT) {
            perror("[Monitor] inotify_add_watch error"); // ERR by zabił cały program
        }
        return;
    }   
    if (add_watch_to_map(wd, root_path) == -1) {
        // jeśli jednak się nie udało dodać do mapy, musimy usunąć watcher.
        inotify_rm_watch(fd, wd);
        return;
    }

    DIR *dir = opendir(root_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char full_path[MAX_PATH_LEN];
        // Sprawdzenie czy ścieżka nie jest za długa
        int n = snprintf(full_path, sizeof(full_path), "%s/%s", root_path, entry->d_name);
        if (n >= MAX_PATH_LEN) {
             fprintf(stderr, "Ścieżka za długa, pomijam: %s/%s\n", root_path, entry->d_name);
             continue;
        }

        struct stat st;
        if (lstat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            add_watch_recursive(fd, full_path);
        }
    }
    closedir(dir);
}