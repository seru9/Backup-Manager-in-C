#include "common.h"
#include "signals.h"
#include "parser.h"
#include "test.h"
#include "file_ops.h"
#include "file_ops_restore.h"
void execute_command(char* input_buffer, pid_source_target pid_archived[], pid_source_target **cursor);
void remove_entry(pid_source_target *array, pid_source_target **cursor, pid_t pid_to_remove);
int main(){
    char input_buffer[MAX_COMMAND_LENGTH];
    static pid_source_target pid_archived[MAX_WATCHERS * MAX_ARGS];
    pid_source_target *cursor = pid_archived;

    sethandler(SIG_IGN, SIGUSR1);
    sethandler(sig_handler, SIGINT);
    sethandler(sig_handler, SIGTERM);
    sethandler(sigchld_handler, SIGCHLD);
    inicjalizujKopiarke();
    printf("Wybierz jedną z funkcji: \n1) add <source_path> <target_paths1> <target_path2> ..\n");
    printf("2) end <source_path> <target_paths1> <target_path2> ..\n3) restore <source_path> <target_path>\n4) list\n");
    printf("5) exit\n");
    while (last_signal != SIGINT && last_signal != SIGTERM) {
        
        // Wczytanie całej linii polecenia
        if (fgets(input_buffer, MAX_COMMAND_LENGTH, stdin) == NULL) {
            if (errno == EINTR) {
                clearerr(stdin); // Wyczyść flagę błędu strumienia
                continue;        // Spróbuj ponownie pobrać linię
            }
            printf("EOF lub SIGINT, kończe program. \n");
            break;
        }
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        execute_command(input_buffer, pid_archived, &cursor);
    }
    printf("\n");
    kill(0, SIGINT);
    while (wait(NULL) > 0);
}

void execute_command(char* input_buffer, pid_source_target pid_archived[], pid_source_target **cursor){

    CommandStruct command_and_paths;
    parse_command(input_buffer, &command_and_paths);

    char command[32];
    char source_path[MAX_PATH_LEN];
    char target_paths[MAX_ARGS][MAX_PATH_LEN];
    int target_count;
    int pid_archived_size = (*cursor) - pid_archived;
    assigner(&command_and_paths, command, source_path, target_paths, &target_count); //przepisz na int'a
    if(check_command(command, source_path, target_paths, target_count) == 0){
        return;
    }
    /*KOMENDA ADD -------------------------------------------*/
    if (strcmp(command, "add") == 0) {
        printf("   [ADD] (Źródło: %s). Uruchamiam kopiowanie dla celów:\n", source_path);
        if(check_path(source_path, 1) == 0){
            printf("[BŁĄD]Ścieżka źródłowa nie istnieje!\n");
            return;
        }
        for (int i = 0; i < target_count; i++) {  
            if(check_path(target_paths[i], 0) == 0)
            {
                printf("  [POMINIĘTO] %s: Katalog źródłowy ma zawartość \n", target_paths[i]);
                continue; //ma zawartość, czyli źle
            } // Jeżeli nie istnieje stworzy, jeżeli istnieje, ale coś jest to skipuje
            if (check_source_target(source_path, target_paths[i], pid_archived,  pid_archived_size) == 0) {
                printf("  [POMINIĘTO] %s: Katalog źródłowy i docelowy są takie same, pomijam.\n", target_paths[i]);
                continue;
            }
            printf("   [ADD] %s\n", target_paths[i]);
            create_child(source_path, target_paths[i], pid_archived, cursor);
        }
        return;
    } 
    /*KOMENDA END -------------------------------------------*/
    else if (strcmp(command, "end") == 0){
        if(check_path(source_path, 1) == 0){
            printf("   [BŁĄD] Ścieżka źródłowa nie istnieje!\n");
            return;
        }
        for(int i = 0; i < target_count; i++){
            pid_t process_to_kill = receive_pid(pid_archived, source_path, target_paths[i]);
            if(process_to_kill == -1){
                printf("   [BŁĄD] Zła ścieżka docelowa! \n");
                return;
            }
            remove_entry(pid_archived, cursor, process_to_kill); // usuwanie z tablicy (funkcja list) naszego procesu
            pid_t k;
            printf("   [END] kończe monitoring dla celu: %s \n", target_paths[i]);
            if((k = kill(process_to_kill, SIGUSR1)) < 0)
                ERR("kill");
        }
        return;
    }
    /*KOMENDA RESTORE -------------------------------------------*/
    else if(strcmp(command, "restore") == 0)    
    {
        char *backup_path = target_paths[0];
        if(check_path(backup_path, 1) == 0){
            printf("   [BŁĄD] Ścieżka backupowa nie istnieje!\n");
            return;
        }
        pid_source_target *iter = pid_archived;
        while (iter < (*cursor)){
            if(strcmp(iter->target_path, backup_path) == 0 && iter->child > 0){
                printf("   [BŁĄD] Sesja monitorowania jest już aktywna dla ścieżki: źródłowej %s | docelowej/backup %s \n", source_path, backup_path);
                return;
            }
            iter++;
        }
        printf("   [Restore] źródło/cel: %s backup: %s\n", source_path, backup_path);
        perform_restore(source_path, backup_path);
        return;
    }
    
    else if (strcmp(command, "list") == 0)
    {
        printf("Listowanie. \n");
        for(int i = 0; i < pid_archived_size; i++){
            printf("   [LIST] Aktywna sesja [%d] ŚCIEŻKA ŹRÓDŁOWA: %s ŚCIEŻKA DOCELOWA %s \n", pid_archived[i].child, pid_archived[i].source_path, pid_archived[i].target_path);
        }
        return;
    }
    // --- 4. Nieznane Polecenie ---
    else if(strcmp(command, "exit") == 0){
        pid_t k;
        if((k = kill(0, SIGTERM)) < 0)
            ERR("kill");
        return;
    }
    printf("Nieznana funkcja, podaj jedną z podanych: \n1) add <source_path> <target_paths1> <target_path2> ..\n");
    printf("2) end <source_path> <target_paths1> <target_path2> ..\n3) restore <source_path> <target_path>\n4) list\n");
    printf("5) exit\n");
    
}
void remove_entry(pid_source_target *array, pid_source_target **cursor, pid_t pid_to_remove) {
    int current_size = *cursor - array; // Obliczamy ile mamy elementów (różnica wskaźników)
    
    for (int i = 0; i < current_size; i++) {
        if (array[i].child == pid_to_remove) {
            
            // 1. Przesuwanie elementów w lewo (nadpisywanie usuwanego)
            // Kopiujemy wszystko od indeksu i+1 do końca o jedno pole wcześniej
            for (int j = i; j < current_size - 1; j++) {
                array[j] = array[j + 1];
            }

            // 2. Cofnięcie kursora (tablica jest teraz o 1 krótsza)
            (*cursor)--;

            // 3. (Opcjonalnie) Wyzerowanie "starego" ostatniego elementu dla czystości
            memset(*cursor, 0, sizeof(pid_source_target));
            
            return; // Zakończ po usunięciu
        }
    }
}
