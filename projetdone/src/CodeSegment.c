#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h> 


#include "../include/CodeSegment.h"

/*
 * Fonction trim
 * Supprime les espaces, tabulations, sauts de ligne et retours chariot en début et fin de chaîne.
 */
char * trim ( char * str ) {
    // Avancer le pointeur tant qu'on a des espaces, tabulations, retours à la ligne ou retours chariot
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r')
        str++;

    // Détermination de la fin de la chaîne en se basant sur strlen
    char * end = str + strlen(str) - 1;
    // Tant que l'on n'est pas arrivé au début et que le caractère actuel est un caractère blanc,
    // on remplace ce caractère par le caractère nul ('\0')
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;  // On décrémente le pointeur pour remonter dans la chaîne.
    }
    return str;
}

/*
 * Fonction search_and_replace
 * Parcourt la table de hachage values et, pour chaque clé,
 * remplace (dans la chaîne *str) toutes les occurrences de cette clé par la valeur correspondante
 * (convertie en chaîne de caractères). Retourne 1 si un remplacement a été effectué, 0 sinon.
 */
int search_and_replace(char **str, HashMap *values) {
    if (!str || !*str || !values)
        return 0;

    int replaced = 0;
    char *input = *str;

    for (int i = 0; i < values->size; i++) {
        if (values->table[i].key && values->table[i].key != (void *)-1) {
            char *key = values->table[i].key;

            // ⚠️ Correction : on récupère correctement un int* et on le convertit en chaîne de caractères
            int *ptr = (int *)values->table[i].value;
            if (!ptr) continue;

            int value = *ptr;

            // Préparer la chaîne de remplacement en tant qu'entier converti en chaîne
            char replacement[64];
            snprintf(replacement, sizeof(replacement), "%d", value);

            // Chercher toutes les occurrences dans input
            char *new_str = NULL;
            char *search_pos = input;
            size_t key_len = strlen(key);
            size_t repl_len = strlen(replacement);

            // Compter combien de fois on doit remplacer
            int count = 0;
            while ((search_pos = strstr(search_pos, key)) != NULL) {
                count++;
                search_pos += key_len;
            }

            if (count == 0) continue;

            // Calculer la taille de la nouvelle chaîne
            size_t new_len = strlen(input) + count * (repl_len - key_len) + 1;
            new_str = malloc(new_len);
            if (!new_str) {
                fprintf(stderr, "Erreur d'allocation mémoire dans search_and_replace.\n");
                exit(EXIT_FAILURE);
            }

            // Construire la nouvelle chaîne avec les remplacements
            char *src = input;
            char *dest = new_str;
            while (*src) {
                if (strncmp(src, key, key_len) == 0) {
                    strcpy(dest, replacement);
                    src += key_len;
                    dest += repl_len;
                    replaced = 1;
                } else {
                    *dest++ = *src++;
                }
            }
            *dest = '\0';

            // Libérer et mettre à jour
            free(input);
            input = new_str;
            *str = new_str;
        }
    }

    // Nettoyage final avec trim
    if (replaced) {
        char *trimmed = trim(input);
        if (trimmed != input) {
            memmove(input, trimmed, strlen(trimmed) + 1);
        }
    }

    return replaced;
}



int resolve_constants(ParserResult *result) {
    if (!result || !result->code_instructions || result->code_count <= 0)
        return -1;

    Instruction **code = result->code_instructions;

    for (int i = 0; i < result->code_count; i++) {
        // Trim des opérandes
        if (code[i]->operand1)
            code[i]->operand1 = trim(code[i]->operand1);
        if (code[i]->operand2)
            code[i]->operand2 = trim(code[i]->operand2);

        // Cas instruction à un seul opérande (labels : JMP, etc.)
        if (code[i]->operand2 == NULL) {
           
            search_and_replace(&code[i]->operand1, result->labels);
          
        }
        else {
            // operand1 peut aussi contenir une variable mémoire
            search_and_replace(&code[i]->operand1, result->memory_locations);

            // Remplacement dans operand2
          
            int replaced = search_and_replace(&code[i]->operand2, result->memory_locations);
          

            // Si on a remplacé ET qu'il n'y a pas déjà de [ ... ], on entoure
            if (replaced) {
                char *val = code[i]->operand2;
                size_t n = strlen(val);
                if (n == 0 || val[0] != '[' || val[n-1] != ']') {
                    char *with_br = malloc(n + 3);
                    if (!with_br) exit(1);
                    with_br[0] = '[';
                    memcpy(with_br+1, val, n);
                    with_br[n+1] = ']';
                    with_br[n+2] = '\0';
                    free(code[i]->operand2);
                    code[i]->operand2 = with_br;
                    printf("→ Encadré : %s\n", code[i]->operand2);
                }
            }
        }
    }

    return 0;
}




void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count) {
    if (!cpu || !cpu->memory_handler || !code_instructions || code_count <= 0) {
        fprintf(stderr, "Erreur : paramètres invalides pour allocate_code_segment.\n");
        return;
    }

    // Étape 1 : créer le segment mémoire "CS" pour le code
    int success = create_segment(cpu->memory_handler, "CS", get_compteur_value(), code_count);
    if (success != 0) {
        fprintf(stderr, "Erreur lors de l'allocation du segment CS.\n");
        return;
    }

    // Étape 2 : stocker les instructions dans CS (chaque instruction dans une case)
    for (int i = 0; i < code_count; i++) {
        if (!store(cpu->memory_handler, "CS", i, code_instructions[i])) {
            fprintf(stderr, "Erreur : stockage de l'instruction %d échoué.\n", i);
        }


    }

    // Étape 3 : initialiser le registre IP à 0 dans le contexte du CPU
    int *ip_value = malloc(sizeof(int));
    if (!ip_value) {
        fprintf(stderr, "Erreur d'allocation mémoire pour IP.\n");
        return;
    }
    *ip_value = 0;
    hashmap_insert(cpu->context, "IP", ip_value);
}
int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest) {
    if (!cpu || !instr) return -1;

    // Résolution systématique des opérandes
    void *resolved_src  = instr->operand2 ? resolve_addressing(cpu, instr->operand2) : NULL;
    
    void *resolved_dest = instr->operand1 ? resolve_addressing(cpu, instr->operand1) : NULL;
    

    if (strcmp(instr->mnemonic, "MOV") == 0) {
        if (resolved_src && resolved_dest) {
            handle_MOV(cpu, resolved_src, resolved_dest);
            printf("=> %s = %d\n", instr->operand1, *(int*)resolved_dest);
        }
    }
    else if (strcmp(instr->mnemonic, "ADD") == 0) {
        if (resolved_src && resolved_dest) {
            *(int*)resolved_dest += *(int*)resolved_src;
            printf("=> %s = %d\n", instr->operand1, *(int*)resolved_dest);
        }
    }
    else if (strcmp(instr->mnemonic, "CMP") == 0) {
        if (resolved_src && resolved_dest) {
            int diff = *(int*)resolved_dest - *(int*)resolved_src;
            printf("%d\n",diff);
            int *ZF = (int*)hashmap_get(cpu->context, "ZF");
            int *SF = (int*)hashmap_get(cpu->context, "SF");
            if (ZF) *ZF = (diff == 0);
            if (SF) *SF = (diff < 0);
            printf("=> ZF = %d, SF = %d\n",
                   ZF ? *ZF : -1,
                   SF ? *SF : -1);
        }
    }
    else if (strcmp(instr->mnemonic, "JMP") == 0) {
        if (resolved_dest) {
            int *IP = (int*)hashmap_get(cpu->context, "IP");
            if (IP) {
                *IP = *(int*)resolved_dest;
                printf("=> IP = %d\n", *IP);
            }
        }
    }
    else if (strcmp(instr->mnemonic, "JZ") == 0) {
        int *ZF = (int*)hashmap_get(cpu->context, "ZF");
        int *IP = (int*)hashmap_get(cpu->context, "IP");
        if (ZF && *ZF == 1 && IP) {
            *IP = *(int*)resolved_dest;
        }
        printf("=> IP = %d (ZF = %d)\n",
               IP  ? *IP  : -1,
               ZF  ? *ZF  : -1);
    }
    else if (strcmp(instr->mnemonic, "JNZ") == 0) {
        int *ZF = (int*)hashmap_get(cpu->context, "ZF");
        int *IP = (int*)hashmap_get(cpu->context, "IP");
        if (ZF && *ZF == 0  && IP) {
            *IP = *(int*)resolved_dest;
        }
        printf("=> IP = %d (ZF = %d)\n",
               IP  ? *IP  : -1,
               ZF  ? *ZF  : -1);
    }
    else if (strcmp(instr->mnemonic, "HALT") == 0) {
        int *IP = (int*)hashmap_get(cpu->context, "IP");
        if (IP) {
            *IP = -1;
            printf("=> HALT, IP = %d\n", *IP);
        }
    }
    else if (strcmp(instr->mnemonic, "PUSH") == 0) {
        
        if (instr->operand1) {
            push_value(cpu, *(int*)resolved_dest);
        }else{
            push_value(cpu, *(int*)hashmap_get(cpu->context, "AX"));
        }
    }
    else if (strcmp(instr->mnemonic, "POP") == 0) {
        
        if (instr->operand1) {
            pop_value(cpu, (int*)resolved_dest);
        }else{
            pop_value(cpu, (int*)hashmap_get(cpu->context, "AX"));
        }
        
    }
    else if (strcmp(instr->mnemonic, "ALLOC") == 0) {
        int rc = alloc_es_segment(cpu);
        if (rc != 0) {
            printf("ALLOC a échoué\n");
        } else {
            printf("ALLOC réussi → ES = %d\n", *(int*)hashmap_get(cpu->context, "ES"));
        }
    }
    else if (strcmp(instr->mnemonic, "FREE") == 0) {
        int rc = free_es_segment(cpu);
        if (rc != 0) {
            printf("FREE a échoué (ES non alloué ?)\n");
        } else {
            printf("FREE réussi → ES libéré\n");
        }
    }

    else {
        return -1;
    }

    return 0;
}

int execute_instruction(CPU *cpu, Instruction *instr) {
    if (!cpu || !instr) {
        return -1; // Vérification de la validité des paramètres
    }

    void *src = NULL;
    void *dest = NULL;

    // Résoudre l'adresse de l'opérande src (opérande 1)
    if (instr->operand2) {
        src = instr->operand2;
        if (!src) {
            fprintf(stderr, "Erreur : l'opérande destination %s n'a pas été trouvé.\n", instr->operand2);
            return -1;
        }
    }

    // Résoudre l'adresse de l'opérande dest (opérande 2)
    if (instr->operand1) {
        dest = instr->operand1;
        if (!dest) {
            fprintf(stderr, "Erreur : l'opérande src %s n'a pas été trouvé.\n", instr->operand1);
            return -1;
        }
    }

    // Exécuter l'instruction avec les opérandes résolus




    return handle_instruction(cpu, instr, src, dest);
}

Instruction* fetch_next_instruction(CPU *cpu){
    int* IP=(int*)hashmap_get(cpu->context,"IP");
    Segment *codeSegment=(Segment *)hashmap_get((cpu->memory_handler)->allocated,"CS");

    if (codeSegment==NULL || (*IP)>=codeSegment->size){

        return NULL;
    }

    Instruction *inst=(Instruction*)load(cpu->memory_handler, "CS",*IP);
    (*IP)++;
    return inst;

}


/*
 * run_program : exécute pas-à-pas le programme chargé dans "CS"
 * - affiche l'état initial (DS + registres)
 * - pour chaque instruction : fetch, affichage, attente Entrée ou 'q', execute
 * - affiche l'état final
 */
void print_data_segment(CPU *cpu) {
    Segment *ds = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
    if (!ds) {
        printf("Segment DS non alloué.\n\n");
        return;
    }
    printf("=== Segment DS (start=%d, size=%d) ===\n", ds->start, ds->size);
    for (int i = ds->start; i < ds->start + ds->size; i++) {
        void *p = cpu->memory_handler->memory[i];
        if (p) {
            printf("  [%2d] = %d\n", i, *(int*)p);
        } else {
            printf("  [%2d] = NULL\n", i);
        }
    }
    printf("\n");
}

void print_registers(CPU *cpu) {
    const char *regs[] = {"AX","BX","CX","DX","IP","ZF","SF","ES"};
    printf("=== Registres ===\n");
    for (size_t i = 0; i < sizeof(regs)/sizeof(*regs); i++) {
        int *v = (int*)hashmap_get(cpu->context, regs[i]);
        printf("  %s = %d\n", regs[i], v ? *v : 0);
    }
    printf("\n");
}

int run_program(CPU *cpu) {
    if (!cpu || !cpu->memory_handler) {
        fprintf(stderr, "run_program: CPU invalide\n");
        return -1;
    }

    printf("\n=== État initial du CPU ===\n\n");
    print_data_segment(cpu);
    print_registers(cpu);

    int *ip = (int*)hashmap_get(cpu->context, "IP");
    if (!ip) {
        fprintf(stderr, "run_program: registre IP introuvable\n");
        return -1;
    }

    while (1) {
        Instruction *instr = fetch_next_instruction(cpu);
        if (!instr) {
            printf("run_program: plus d'instructions ou IP hors limites (%d)\n", *ip);
            break;
        }

        printf("IP=%d : %s", *ip, instr->mnemonic);
        if (instr->operand1) printf(" %s", instr->operand1);
        if (instr->operand2) printf(", %s", instr->operand2);
        printf("\n");

        // Attente de l'entrée utilisateur
        printf("Appuyez sur Entrée pour continuer ou 'q' puis Entrée pour quitter...\n");
        int c = getchar();
        if (c == 'q') {
            printf("Exécution interrompue par l'utilisateur.\n");
            break;
        }
        // Vider le buffer jusqu’à '\n' si autre que 'q'
        while (c != '\n' && c != EOF) c = getchar();

        if (execute_instruction(cpu, instr) != 0) {
            fprintf(stderr, "run_program: échec exécution à IP=%d\n", *ip);
            break;
        }
    }

    printf("\n=== État final du CPU ===\n\n");
    print_data_segment(cpu);
    print_registers(cpu);

    return 0;
}

