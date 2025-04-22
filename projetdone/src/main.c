#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h> 


#include "../include/CodeSegment.h"



void afficherHashMap(HashMap *map) {
    if (map == NULL) {
        printf("La HashMap est NULL.\n");
        return;
    }
    printf("Taille de la table : %d\n", map->size);
    for (int i = 0; i < map->size; i++) {
        if (map->table[i].key != NULL && map->table[i].key != ((void *)-1))
 {
            // On suppose que les valeurs sont des int*.
            int *val = (int *)map->table[i].value;
            printf("  Clé : %s, Valeur : %d\n", map->table[i].key, (val ? *val : 0));
        }
    }
    printf("\n");
}

// Fonction utilitaire : affiche les tables des labels et des emplacements mémoire
void afficherResultats(ParserResult *result) {
    printf("==== Table des Labels ====\n");
    afficherHashMap(result->labels);
    printf("==== Table des Memory Locations ====\n");
    afficherHashMap(result->memory_locations);
}
static void test_run_program_existing(void) {
    printf("=== test_run_program_existing ===\n");

    // 1) Parser le fichier lina.txt
    ParserResult *res = parse("test.txt");
    assert(res && "Échec du parse de lina.txt");

    // 2) Calcul de la taille nécessaire (DATA + CODE + SS + un peu pour ES)
    int ds_size = get_compteur_value();    // nb d'éléments DATA
    int cs_size = res->code_count;         // nb d'instructions CODE
    int mem_size = ds_size + cs_size + 120+ 100;  // +100 pour ES

    // 3) Initialiser le CPU
    CPU *cpu = cpu_init(mem_size);
    assert(cpu && "Échec de cpu_init");

    // 4) Allouer et remplir DS
    allocate_variables(cpu, res->data_instructions, res->data_count);

    // 5) Résoudre labels et constantes
    resolve_constants(res);

    // 6) Allouer CS et y stocker le code
    allocate_code_segment(cpu, res->code_instructions, res->code_count);

    // 7) Vérifier CS[1] == MOV BX, 6
    {
        Instruction *ins1 = load(cpu->memory_handler, "CS", 1);
        assert(ins1 && "Aucune instruction à CS[1]");
        assert(strcmp(ins1->mnemonic, "MOV") == 0);
        assert(strcmp(ins1->operand1, "BX") == 0);
        assert(strcmp(ins1->operand2, "6") == 0);
        printf("✅ CS[1] = MOV BX, 6\n");
    }

    // 8) Lancer l'exécution (consomme EOF, pas d'interaction)
    printf("\n-- run_program --\n");
    run_program(cpu);

    // 9) Vérifier l'état final
    int final_ax = *(int*)hashmap_get(cpu->context, "AX");
    int final_bx = *(int*)hashmap_get(cpu->context, "BX");
    int final_ip = *(int*)hashmap_get(cpu->context, "IP");

    //  - après MOV AX,[ES:BX] où CX=42 → AX doit valoir 42
    //  - ALLOC avait mis BX à 0 (premier offset libre)
    //  - FREE ne change pas BX
    //  - IP doit atteindre code_count en sortie
    assert(final_ax == 42);
    assert(final_bx == 0);
    assert(final_ip == res->code_count);
    printf("✅ Après run_program : AX = %d, BX = %d, IP = %d\n",
           final_ax, final_bx, final_ip);

    // 10) Cleanup
    free_parser_result(res);
    cpu_destroy(cpu);

    printf("✅ test_run_program_existing passed\n\n");
}



// -----------------------------------
// main
// -----------------------------------
int main(void) {
    // Vos tests précédents...
    test_run_program_existing();

    return 0;
}