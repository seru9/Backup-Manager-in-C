#include "signals.h"
#include "file_ops.h"// Potrzebne do make_copy, resolve_absolute_path
#include "inotify.h"
// Definicja zmiennej globalnej (tylko w jednym pliku .c)
volatile sig_atomic_t last_signal = 0;

void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if(sigNo == SIGCHLD)
        act.sa_flags = SA_RESTART;  
    if (-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");
}


void sig_handler(int sig) {
    last_signal = sig;
}

void sigchld_handler() {
    pid_t pid;
    while (1) {
        pid = waitpid(0, NULL, WNOHANG);
        if (pid == 0) return;
        if (pid <= 0) {
            if (errno == ECHILD) return;
            ERR("waitpid");
        }
    }
}

int should_stop() {
    return (last_signal == SIGUSR1 || last_signal == SIGTERM || last_signal == SIGINT);
}

pid_t receive_pid(pid_source_target *tablica, const char *source_path, const char *target_path) {

    for (int i = 0; i < MAX_WATCHERS * MAX_ARGS; i++) {
        if (tablica[i].child != 0 &&
            strcmp(tablica[i].source_path, source_path) == 0 &&
            strcmp(tablica[i].target_path, target_path) == 0) {
            return tablica[i].child;
        }
    }
    return -1;
}
void create_child(char source_path[], char target_path[], pid_source_target pid_archived[], pid_source_target** cursor) {
    if ((*cursor) - pid_archived >= MAX_WATCHERS * MAX_ARGS) {
        fprintf(stderr, "Błąd: Osiągnięto limit procesów potomnych!\n");
        return;
    }
    pid_t pid;
    if ((pid = fork()) < 0)
        ERR("fork");

    if (0 == pid) {
        sethandler(sig_handler, SIGUSR1);
        child_work(source_path, target_path);
        exit(EXIT_SUCCESS);
    } else {
        if ((*cursor) - pid_archived < MAX_WATCHERS * MAX_ARGS) {
            (*cursor)->child = pid;
            snprintf((*cursor)->source_path, MAX_PATH_LEN, "%s", source_path);
            snprintf((*cursor)->target_path, MAX_PATH_LEN, "%s", target_path);
            (*cursor)++;
        }
        return;
    }
}
void child_work(char source_path[], char target_path[]) {
    make_copy(source_path, target_path);
    printf("   [INotify] Aktywuje monitorowanie aktywne: %s -> %s\n", source_path, target_path);
    int fd;
    if((fd = inotify_init()) < 0){
        ERR("inotify_init");
    }
    watches_count = 0; 
    memset(&watches, 0, sizeof(watches)); // może być źle
    add_watch_recursive(fd, source_path);

    char buffer[8192] __attribute__ ((aligned(__alignof__(struct inotify_event))));

    while (!should_stop()) {
        ssize_t len = read(fd, buffer, sizeof(buffer)); 
        
        if (len < 0) {
            if (errno == EINTR) continue; 
            perror("read inotify error"); //ERR by zabił cały program!!!!
            sleep(1);
            continue;
        }


        char *ptr = buffer;
        while (ptr < buffer + len) {
            struct inotify_event *event = (struct inotify_event *)ptr;
            if (event->len > 0) {

                if (event->name[0] == '.' || strstr(event->name, "..")) {
                    ptr += sizeof(struct inotify_event) + event->len;
                    continue;
                }
                const char *dir_path = get_path_by_wd(event->wd);
                
                if (dir_path != NULL) {
                    char src_full[MAX_PATH_LEN];
                    char dst_full[MAX_PATH_LEN];

                    // Budowanie ścieżek
                    int written_src = snprintf(src_full, sizeof(src_full), "%s/%s", dir_path, event->name);
                    // Obliczanie ścieżki docelowej
                    int src_root_len = strlen(source_path);
                    // dir_path to np. "/home/user/src/podkatalog". 
                    // dir_path + src_root_len daje "/podkatalog"
                    int written_dst = snprintf(dst_full, sizeof(dst_full), "%s%s/%s", 
                             target_path, 
                             dir_path + src_root_len, 
                             event->name);
                    if (written_src < MAX_PATH_LEN && written_dst < MAX_PATH_LEN) {
                        if ((event->mask & IN_CREATE) || (event->mask & IN_MOVED_TO)) {
                            
                            if (event->mask & IN_ISDIR) {
                                printf("Wykryto nowy katalog: %s\n", src_full);
                                add_watch_recursive(fd, src_full);
                                make_copy(src_full, dst_full);
                            } else {
                                printf("Wykryto nowy plik: %s\n", src_full);
                                copy_reg_file(src_full, dst_full);
                            }
                        }
                        
                        // Plik usunięty lub wyniesiony
                        else if ((event->mask & IN_DELETE) || (event->mask & IN_MOVED_FROM)) {
                            printf("Usuwanie: %s\n", dst_full);
                            remove_recursive(dst_full);
                        }
                        
                        // Modyfikacja
                        else if ((event->mask & IN_MODIFY) || (event->mask & IN_ATTRIB) || (event->mask & IN_CLOSE_WRITE)) {
                            if (!(event->mask & IN_ISDIR)) {
                                printf("Wykryto nową modyfikacje (przy CREATE też sie pojawia): %s\n", src_full);
                                copy_reg_file(src_full, dst_full); 
                            }
                        }
                    }
                }
                else{
                    printf("  -> BŁĄD: Nie znaleziono ścieżki dla WD [%d]!\n", event->wd);
                }
            }

            // Przesunięcie wskaźnika do następnego zdarzenia
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }
    printf("   [INOTIFY] Koniec monitoringu dla źrodło: %s docelowy: %s\n", source_path, target_path);
    close(fd);
}