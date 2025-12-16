#include "test.h"
#include <stdio.h>


#include <stdio.h>
#include "test.h"

// Funkcja przyjmuje poszczególne pola zamiast struktury
void print_command_data(int is_valid, const char *command, const char *source_path, const char target_paths[][MAX_PATH_LEN],int target_count)
{
    printf("========== COMMAND DEBUG ==========\n");
    
    // Print Validation Status
    printf("Is Valid:     %s (%d)\n", is_valid ? "YES" : "NO", is_valid);

    // Print Command and Source
    // Używamy bezpiecznych formatów stringów
    printf("Command:      '%s'\n", command ? command : "NULL");
    printf("Source Path:  '%s'\n", source_path ? source_path : "NULL");
    
    // Print Targets
    printf("Target Count: %d\n", target_count);
    
    if (target_count > 0) {
        printf("Target Paths:\n");
        for (int i = 0; i < target_count; i++) {
            // target_paths[i] zadziała poprawnie dzięki deklaracji [][MAX_PATH_LEN]
            printf("\t[%d]: '%s'\n", i, target_paths[i]);
        }
    } else {
        printf("Target Paths: [None]\n");
    }
    
    printf("===================================\n");
}
void print_watches() {
    printf("Liczba obserwowanych katalogów: %d\n", watches_count);
    for (int i = 0; i < watches_count; i++) {
        printf("  Indeks %d: WD [%d] -> Path [%s]\n", i, watches[i].wd, watches[i].path);
    }
}
#include <stdio.h>

void inicjalizujKopiarke() {
    printf("\n");
    printf("         _________________________\n");
    printf("        |    [ KOPIOWANIE ]     |\n");
    printf("        |_______________________|\n");
    printf("        |  [Start] [Stop] [?]   |\n");
    printf("        |_______________________|\n");
    printf("            |_______________|\n");
    printf("           /===============/    ERROR: 404\n");
    printf("          /===============/     TONER LOW\n");
    printf("         /_______________/      WILL TO LIVE LOW\n");
    printf("         |               |\n");
    printf("         |   PAPER JAM   |\n");
    printf("         |_______________|\n");
    printf("\n");
    printf("   > System: Kopiarka gotowa (zeby sie zepsuc).\n");
    printf("==================================================\n\n");
}
