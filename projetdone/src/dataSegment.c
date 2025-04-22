
#include <regex.h>
#include <assert.h>
#define STACK_SIZE 128
#define STACK_SIZE 128

#include "../include/dataSegment.h"

#define STACK_SIZE 128

CPU* cpu_init(int memory_size) {
    if (memory_size < STACK_SIZE) return NULL;

    CPU* cpu = malloc(sizeof(CPU));
    if (!cpu) return NULL;

    cpu->memory_handler   = memory_init(memory_size);
    cpu->context          = hashmap_create();
    cpu->constant_pool    = hashmap_create();

    // Registres généraux et drapeaux
    int *ax = malloc(sizeof(int)); *ax = 0;  hashmap_insert(cpu->context, "AX", ax);
    int *bx = malloc(sizeof(int)); *bx = 0;  hashmap_insert(cpu->context, "BX", bx);
    int *cx = malloc(sizeof(int)); *cx = 0;  hashmap_insert(cpu->context, "CX", cx);
    int *dx = malloc(sizeof(int)); *dx = 0;  hashmap_insert(cpu->context, "DX", dx);
    int *ip = malloc(sizeof(int)); *ip = 0;  hashmap_insert(cpu->context, "IP", ip);
    int *zf = malloc(sizeof(int)); *zf = 0;  hashmap_insert(cpu->context, "ZF", zf);
    int *sf = malloc(sizeof(int)); *sf = 0;  hashmap_insert(cpu->context, "SF", sf);
    int *es = malloc(sizeof(int)); *es = -1;  hashmap_insert(cpu->context, "ES", es);

    // Registres de pile
    int *sp = malloc(sizeof(int)); *sp = memory_size;       hashmap_insert(cpu->context, "SP", sp);
    int *bp = malloc(sizeof(int)); *bp = memory_size;       hashmap_insert(cpu->context, "BP", bp);

    // Création du segment de pile SS
    create_segment(cpu->memory_handler,
                   "SS",
                   memory_size - STACK_SIZE,
                   STACK_SIZE);

    return cpu;
}



void cpu_destroy(CPU* cpu) {
    if (cpu == NULL) {
        return;
    }
    if (cpu->memory_handler != NULL) {
        destroy_memory_handler(cpu->memory_handler);
    }
    if (cpu->context != NULL) {
        hashmap_destroy(cpu->context);
    }
    if (cpu->constant_pool != NULL) {
        hashmap_destroy(cpu->constant_pool);
    }
    free(cpu);
}

void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data) {
    Segment* seg = hashmap_get(handler->allocated, segment_name);
    if (!seg) return NULL;
    // pos doit être dans [0 .. seg->size-1]
    if (pos < 0 || pos >= seg->size) return NULL;

    int absolute = seg->start + pos;
    handler->memory[absolute] = data;
    // printf("data inseree a la position %d\n", absolute);  // si vous voulez du debug
    return data;
}

void* load(MemoryHandler *handler, const char *segment_name, int pos) {
    Segment* seg = hashmap_get(handler->allocated, segment_name);
    if (!seg) return NULL;
    if (pos < 0 || pos >= seg->size) return NULL;
    return handler->memory[seg->start + pos];
}

void allocate_variables(CPU *cpu, Instruction** data_instructions,int data_count){
if (hashmap_get(cpu->memory_handler->allocated, "DS")){
hashmap_remove(cpu->memory_handler->allocated, "DS");
}

create_segment(cpu->memory_handler, "DS", 0, get_compteur_value());
int index = 0;
for (int i = 0; i < data_count; i++) {
    Instruction *inst = data_instructions[i];
    char *operand2 = inst->operand2;

    // Tokenisation de operand2 pour extraire les entiers
    char *token = strtok(operand2, "',");
    /*########################################################################################################*/
    /*ATAGHYOUUULTint index = 0;*/
    /*########################################################################################################*/
    // Tableau pour stocker les pointeurs vers les entiers
    while (token != NULL) {
        int value = atoi(token);  // Convertir le token en entier

        // Allocation de mémoiint* value=malloc(sizeof(int));
        cpu->memory_handler->memory[index] =(int *) malloc(sizeof(int));
        *(int *)cpu->memory_handler->memory[index] = value;  // Stocker la valeur
        index++;  // Incrémenter l'index pour la prochaine position mémoire
        token = strtok(NULL, "',");  // Passer au token suivant
    }
}
}

void print_data(CPU *cpu) {
    MemoryHandler *handler = cpu->memory_handler;
    // Récupération du segment DS dans le gestionnaire de mémoire
    Segment *ds = hashmap_get(handler->allocated, "DS");
    
    if (ds == NULL) {
        printf("Le segment DS n'existe pas.\n");
        return;
    
    int start = ds->start;
    int size = ds->size;  // Nombre total d'emplacements alloués pour DS
    
    printf("Contenu du segment DS (positions %d a %d) :\n", start, start + size - 1);
    
    for (int i = start; i < start + size; i++) {
        // Vérifier que l'emplacement contient une donnée allouée
        if (handler->memory[i] != NULL) {
            // Supposons que chaque emplacement contient un pointeur sur un int
            int value = *(int *)handler->memory[i];
            printf("memory[%d] = %d\n", i, value);
        } else {
            printf("memory[%d] = NULL\n", i);
        }
    }
}}

int matches ( const char* pattern , const char* string ) {
    regex_t regex ;
    int result = regcomp (&regex , pattern,REG_EXTENDED) ;
    if ( result ) {
    fprintf (stderr ,"  Regex c om pil a ti o n f a i l e d f o r p a t t e r n : %s \n", pattern ) ;
    return 0;
    }
    result = regexec (&regex , string , 0 , NULL , 0) ;
    regfree (&regex ) ;
    return result == 0;
     }
 
    void* immediate_adressing(CPU* cpu, const char* operand){
    if (!matches("^-?[0-9]+$",operand)){
        return NULL;
    }
    void* existing=hashmap_get(cpu->constant_pool, operand);
    if (existing!=NULL){
        return existing;
    }
   
    int* value=malloc(sizeof(int));
    *value=atoi(operand);
    char*key=strdup(operand);
    hashmap_insert(cpu->constant_pool,key,value);
    return value;
        
    }
    void* register_adressing(CPU* cpu, const char* operand) {
    if (!matches("^(AX|BX|CX|DX)$", operand)) {
        return NULL;
    }

    void* reg_value = hashmap_get(cpu->context, operand);
    if (reg_value != NULL) {
        return reg_value;
    }

    return NULL;  // Le registre n'existe pas (ça ne devrait jamais arriver si bien initialisé)
}

    void* memory_direct_adressing(CPU* cpu, const char* operand) {
    if (!matches("^\\[[0-9]+\\]$", operand)) {
        return NULL;
    }

    // Extraire le nombre entre les crochets
    int address;
    sscanf(operand, "[%d]", &address);

    // Vérifie que l'adresse est dans les limites
    if (address < 0 || address >= cpu->memory_handler->total_size) {
        return NULL;
    }

    return cpu->memory_handler->memory[address];
}

void *register_indirect_addressing(CPU *cpu, const char *operand) {
    // Vérifier que l'opérande respecte bien le format "[XX]" où XX sont deux lettres majuscules.
    if (!matches("^\\[[A-Z]{2}\\]$", operand)) {
        return NULL;
    }
    
    char registre[10] = {0};  // Zéro-initialisation pour garantir '\0'
    if (sscanf(operand, "[%2[A-Z]]", registre) != 1) {
        return NULL;
    }
    registre[2] = '\0'; // <- sécurise même si sscanf ne lit qu’1 caractère
    

    


    return hashmap_get(cpu->context, registre);
}


void handle_MOV(CPU* cpu, void* src, void* dest) {

    if (!src || !dest) {
        fprintf(stderr, "handle_MOV: pointeur NULL détecté\n");
        return;
    }
    
    // Copie de la valeur depuis 'src' vers 'dest'.
    // On effectue un cast sur les pointeurs 'src' et 'dest' en 'int*' car ils
    // pointent vers des données de type int. Ensuite, on déférence ces pointeurs
    // afin d'accéder aux valeurs et de les copier.
    *(int*)dest = *(int*)src;
}
void* segment_override_addressing(CPU* cpu, const char* operand) {
    printf("operande:%s",operand);
    // 1) Validation syntaxique : [XX:YY]
    if (!matches("^\\[[A-Z]{2}:[A-Z]{2}\\]$", operand)) {
        
        return NULL;
    }
    printf("ikemmel");
    // 2) Extraction des deux groupes, exactement 2 lettres majuscules
    char seg_name[3] = {0};
    char reg_name[3] = {0};
    // Literal '[', then %2[A-Z] (2 lettres majuscules), literal ':',
    // then %2[A-Z], literal ']'
    if (sscanf(operand, "[%2[A-Z]:%2[A-Z]]", seg_name, reg_name) != 2) {
        printf("[%s:%s]",seg_name,reg_name) ;
        return NULL;
    }
  

    // 3) Lookup du segment
    Segment *seg = hashmap_get(cpu->memory_handler->allocated, seg_name);
    if (!seg) 
    { 
        return NULL;}

    // 4) Lookup du registreint free_es_segment(CPU *cpu) (valeur → offset)
    int *reg_val = hashmap_get(cpu->context, reg_name);
    if (!reg_val) return NULL;

    int offset = *reg_val;

    // 5) Vérification des bornes
    if (offset < 0 || offset >= seg->size) {
        return NULL;
    }

    // 6) Retourne la donnée stockée à seg->start + offset
    return cpu->memory_handler->memory[seg->start + offset];
}


#include <stdio.h>

// Résout l'opérande en affichant le type d'adressage utilisé
void* resolve_addressing(CPU *cpu, const char *operand) {
    void* result = NULL;

    // 1. Adressage immédiat
    result = segment_override_addressing(cpu,operand);
    if (result != NULL) {
        printf("[resolve] \"%s\" via addressing override addressing → %p\n", operand, result);
        return result;
    }


    result = immediate_adressing(cpu, operand);
    if (result != NULL) {
        printf("[resolve] \"%s\" via addressing immédiat → %p\n", operand, result);
        return result;
    }

    // 2. Adressage par registre
    result = register_adressing(cpu, operand);
    if (result != NULL) {
        printf("[resolve] \"%s\" via addressing registre   → %p\n", operand, result);
        return result;
    }

    // 3. Adressage direct en mémoire
    result = memory_direct_adressing(cpu, operand);
    if (result != NULL) {
        printf("[resolve] \"%s\" via addressing direct mémoire → %p\n", operand, result);
        return result;
    }

    // 4. Adressage indirect par registre
    result = register_indirect_addressing(cpu, operand);
    if (result != NULL) {
        printf("[resolve] \"%s\" via addressing indirect registre → %p\n", operand, result);
        return result;
    }

    // Aucun mode n'a fonctionné
    printf("[resolve] \"%s\" : aucun mode applicable\n", operand);
    return NULL;
}

int alloc_es_segment(CPU *cpu) {
    if (!cpu) return -1;

    int *ax = hashmap_get(cpu->context, "AX");  
    int *bx = hashmap_get(cpu->context, "BX");  
    int *zf = hashmap_get(cpu->context, "ZF");
    int *es = hashmap_get(cpu->context, "ES");

    if (!ax || !bx || !zf || !es) {
        fprintf(stderr, "alloc_es_segment: registres manquants.\n");
        return -1;
    }

    int taille = *ax;
    int strategie = *bx;

    int start = find_free_address_strategy(cpu->memory_handler, taille, strategie);
    if (start == -1) {
        *zf = 1; // échec
        return -1;
    }

    if (create_segment(cpu->memory_handler, "ES", start, taille) != 0) {
        *zf = 1;
        return -1;
    }

    // Initialisation à zéro
    for (int i = 0; i < taille; i++) {
        int *zero = malloc(sizeof(int));
        *zero = 0;
        cpu->memory_handler->memory[start + i] = zero;
    }

    *es = start;
    *zf = 0; // succès

    return 0;
}

int free_es_segment(CPU *cpu) {
    if (!cpu) return -1;

    // 1. Récupérer le registre ES
    int *es = hashmap_get(cpu->context, "ES");
    if (!es || *es == -1) {
        // Segment ES déjà libéré ou jamais alloué
        return -1;
    }

    // 2. Récupérer le segment ES dans la mémoire allouée
    Segment *es_seg = hashmap_get(cpu->memory_handler->allocated, "ES");
    if (!es_seg) {
        return -1;
    }

    // 3. Libérer chaque cellule de mémoire du segment
    for (int i = 0; i < es_seg->size; i++) {
        int addr = es_seg->start + i;
        if (cpu->memory_handler->memory[addr]) {
            free(cpu->memory_handler->memory[addr]);
            cpu->memory_handler->memory[addr] = NULL;
        }
    }

    // 4. Supprimer le segment de la table des segments
    hashmap_remove(cpu->memory_handler->allocated, "ES");

    // 5. Réinitialiser le registre ES à -1
    *es = -1;

    return 0;
}


