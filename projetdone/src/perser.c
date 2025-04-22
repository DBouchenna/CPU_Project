
#include "../include/perser.h"



static int compteur = 0;
Instruction *parse_data_instruction(const char *line, HashMap *memory_locations) {
    Instruction *inst = malloc(sizeof(Instruction));
    if (!inst) return NULL;

    char var_name[100] = "", type[100] = "", value[255] = "";
    int n = sscanf(line, "%99s %99s %[^\n]", var_name, type, value);
    if (n < 3) {
        free(inst);
        return NULL;
    }

    inst->mnemonic = strdup(var_name);
    inst->operand1 = strdup(type);
    inst->operand2 = strdup(value);

    // On compte les éléments (séparés par des virgules)
    int nb_elements = 0;
    char *p = value;
    while (*p) {
        if (*p == ',') nb_elements++;
        p++;
    }

    if (value[0] != '\0') nb_elements++; // au moins une valeur présente

    // On stocke l'adresse de la variable dans memory_locations
    int *addr = malloc(sizeof(int));
    *addr = compteur;
    hashmap_insert(memory_locations, var_name, addr);

    // On incrémente le compteur global d’adresses
    compteur += nb_elements;

    return inst;
}

int get_compteur_value() {
    
    return compteur;  // Retourne la valeur actuelle de 'compteur'
}

Instruction *parse_code_instruction(const char *line, HashMap *labels, int code_count) {
    // Allocation dynamique pour stocker l'instruction
    Instruction *inst = malloc(sizeof(Instruction));
    if (!inst) return NULL;  // Vérification si malloc a échoué

    

    // Buffers pour extraire les parties de l'instruction
    char label[50]    = "";
    char opcode[150]   = "";
    char operand1[150] = "";
    char operand2[150] = "";

    int n = 0;  // Variable pour stocker le nombre de champs lus

    /*** CAS 1 : LABEL + OPCODE + OPERANDE1,OPERANDE2 ***/
    n = sscanf(line, "%49[^:]: %49s %49[^,],%49s", label, opcode, operand1, operand2);

    if (n == 4) {
 
        int *value = malloc(sizeof(int));
        *value = code_count;
        hashmap_insert(labels, label, value);
        
        
        inst->mnemonic = strdup(opcode);
        inst->operand1 = strdup(operand1);
        inst->operand2 = strdup(operand2);
        return inst;
    }

    /*** CAS 2 : LABEL + OPCODE + OPERANDE1 (PAS DE DEUXIÈME OPERANDE) ***/
    n = sscanf(line, "%49[^:]: %49s %49s", label, opcode, operand1);
    if (n == 3) {
         int *value = malloc(sizeof(int));
        *value = code_count;
        hashmap_insert(labels, label, value);
        inst->mnemonic = strdup(opcode);
        inst->operand1 = strdup(operand1);
        inst->operand2 = NULL;  // Pas de deuxième opérande
        return inst;
    }

    /*** CAS 3 : OPCODE + OPERANDE1,OPERANDE2 (PAS DE LABEL) ***/
    n = sscanf(line, "%49s %49[^,],%49s", opcode, operand1, operand2);
    if (n == 3) {
      
        inst->mnemonic = strdup(opcode);
        inst->operand1 = strdup(operand1);
        inst->operand2 = strdup(operand2);
        return inst;
    }

    /*** CAS 4 : OPCODE + OPERANDE1 (PAS DE LABEL, PAS DE DEUXIÈME OPERANDE) ***/
    n = sscanf(line, "%49s %49s", opcode, operand1);
    if (n == 2) {

 
        inst->mnemonic = strdup(opcode);
        inst->operand1 = strdup(operand1);
        inst->operand2 = NULL;
        return inst;
    }


// Cas spécial ALLOC
if (strncmp(line, "ALLOC", 5) == 0) {
    inst->mnemonic = strdup("ALLOC");
    inst->operand1 = NULL;
    inst->operand2 = NULL;
    return inst;
}

// Cas spécial FREE
if (strncmp(line, "FREE", 4) == 0) {
    inst->mnemonic = strdup("FREE");
    inst->operand1 = NULL;
    inst->operand2 = NULL;
    return inst;
}
    /*** AUCUN FORMAT RECONNU : LIBÉRER LA MÉMOIRE ET RETOURNER NULL ***/
    free(inst);
    return NULL;
}


// Fonction parse
ParserResult *parse(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return NULL;
    }

    // Initialisation de ParserResult
    ParserResult *result = malloc(sizeof(ParserResult));
    result->data_instructions = NULL;
    result->data_count = 0;
    result->code_instructions = NULL;
    result->code_count = 0;
    result->labels = hashmap_create();
    result->memory_locations = hashmap_create();

    char line[256];
    int in_data_section = 0, in_code_section = 0;

    // Lecture ligne par ligne
    while (fgets(line, sizeof(line), file)) {
        // Nettoyage de la ligne (supprime les sauts de ligne)
        
        
        // Détecter la section actuelle
        if (strncmp(line, ".DATA",5) == 0) {
            
            in_data_section = 1;
            in_code_section = 0;
            continue;
        } 
        if (strncmp(line, ".CODE",5) == 0) {
            in_data_section = 0;
            in_code_section = 1;
            
            continue;
        }

        // Traitement des instructions DATA
        if (in_data_section) {
            Instruction *inst = parse_data_instruction(line, result->memory_locations);
            if (inst) {
                result->data_count++;
                result->data_instructions = realloc(result->data_instructions, result->data_count * sizeof(Instruction *));
                result->data_instructions[result->data_count - 1] = inst;
            }
        }

        // Traitement des instructions CODE
        else if (in_code_section) {
            Instruction *inst = parse_code_instruction(line, result->labels, result->code_count);
            if (inst) {
                result->code_count++;
                result->code_instructions = realloc(result->code_instructions, result->code_count * sizeof(Instruction *));
                result->code_instructions[result->code_count - 1] = inst;
            }
        }
    }

    fclose(file);
    return result;
}



void free_parser_result(ParserResult *result) {
    if (!result) return;  // Check for NULL pointer

    // Free all .DATA instructions
    if (result->data_instructions) {
        for (int i = 0; i < result->data_count; i++) {
            free(result->data_instructions[i]);  // Free each instruction
        }
        free(result->data_instructions);  // Free the array itself
    }

    // Free all .CODE instructions
    if (result->code_instructions) {
        for (int i = 0; i < result->code_count; i++) {
            free(result->code_instructions[i]);  // Free each instruction
        }
        free(result->code_instructions);  // Free the array itself
    }

    // Free the labels hash map if necessary
    if (result->labels) {
        hashmap_destroy(result->labels);  // Assuming a proper function exists
    }

    // Free the memory locations hash map if necessary
    if (result->memory_locations) {
        hashmap_destroy(result->memory_locations);  // Assuming a proper function exists
    }

    // Free the ParserResult structure itself
    free(result);
}
void afficherInstructions(Instruction *tableau[], int taille) {
    for (int i = 0; i < taille; i++) {
        if (tableau[i] != NULL) {
            printf("Instruction %d:\n", i + 1);
            printf("  Mnemonic : %s\n", tableau[i]->mnemonic);
            printf("  Operand1 : %s\n", tableau[i]->operand1);
            printf("  Operand2 : %s\n\n", tableau[i]->operand2);
        }
    }
}



/*int main(){
    HashMap *hashtab=hashmap_create();
    Instruction *in=parse_data_instruction("X vb '12,8,9'", hashtab);
    Instruction *in3=parse_data_instruction("a vb '12,8,9'", hashtab);
    ///Instruction *in4=parse_data_instruction("arr DB '20, 21, 22, 23'", hashtab);
    int compteur=(int)hashmap_get(hashtab,"a");
    
    Instruction *in2=parse_code_instruction("ax b", hashtab, 0);
    ParserResult  *l=parse("lina.txt");
    int a=(int)hashmap_get(l->labels, "loop"); 
    printf("%d",a);
    free_parser_result(l);
    int a2=(int)hashmap_get(l->labels, "loop"); 
    printf("%d",a2);
    ///printf("%s ,%s,%s",in2->mnemonic,in2->operand1,in2->operand2);
  
}*/

