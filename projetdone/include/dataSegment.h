#include "gestion_memoire.h"

// Structure représentant un CPU avec ses composants principaux
typedef struct {
    MemoryHandler *memory_handler;  // Gestionnaire de mémoire
    HashMap *context;              // Registres (AX, BX, CX, DX, IP, etc.)
    HashMap *constant_pool;        // Pool de constantes (pour les valeurs immédiates)
} CPU;

/**
 * @brief Initialise un CPU avec un gestionnaire de mémoire et des registres.
 *
 * @param memory_size Taille totale de la mémoire du CPU.
 * @return CPU* Pointeur vers l'instance initialisée du CPU.
 */
CPU *cpu_init(int memory_size);

/**
 * @brief Détruit une instance de CPU en libérant la mémoire associée.
 *
 * @param cpu Pointeur vers l'instance du CPU à détruire.
 */
void cpu_destroy(CPU *cpu);

/**
 * @brief Sauvegarde des données dans un segment mémoire spécifique.
 *
 * @param handler Gestionnaire de mémoire.
 * @param segment_name Nom du segment mémoire.
 * @param pos Position dans le segment.
 * @param data Donnée à stocker.
 * @return void* Retourne un pointeur vers les données stockées.
 */
void *store(MemoryHandler *handler, const char *segment_name, int pos, void *data);

/**
 * @brief Charge des données depuis un segment mémoire spécifique.
 *
 * @param handler Gestionnaire de mémoire.
 * @param segment_name Nom du segment mémoire.
 * @param pos Position dans le segment.
 * @return void* Pointeur vers les données chargées.
 */
void *load(MemoryHandler *handler, const char *segment_name, int pos);

/**
 * @brief Alloue les variables du segment "DS" (Data Segment).
 *
 * @param cpu Pointeur vers le CPU.
 * @param data_instructions Instructions à allouer dans la mémoire.
 * @param data_count Nombre d'instructions à allouer.
 */
void allocate_variables(CPU *cpu, Instruction **data_instructions, int data_count);

/**
 * @brief Affiche les données présentes dans le segment "DS".
 *
 * @param cpu Pointeur vers le CPU.
 */
void print_data(CPU *cpu);

/**
 * @brief Fonction utilitaire pour vérifier si une chaîne correspond à un motif regex.
 *
 * @param pattern Motif regex.
 * @param string Chaîne à tester.
 * @return int 1 si correspondance, 0 sinon.
 */
int matches(const char *pattern, const char *string);

/**
 * @brief Gestion de l'adressage immédiat (valeur littérale).
 *
 * @param cpu Pointeur vers le CPU.
 * @param operand Opérande à analyser.
 * @return void* Pointeur vers la valeur correspondante.
 */
void *immediate_adressing(CPU *cpu, const char *operand);

/**
 * @brief Gestion de l'adressage par registre (AX, BX, etc.).
 *
 * @param cpu Pointeur vers le CPU.
 * @param operand Opérande à analyser.
 * @return void* Pointeur vers la valeur du registre.
 */
void *register_adressing(CPU *cpu, const char *operand);

/**
 * @brief Gestion de l'adressage direct en mémoire.
 *
 * @param cpu Pointeur vers le CPU.
 * @param operand Opérande à analyser.
 * @return void* Pointeur vers la valeur mémoire.
 */
void *memory_direct_adressing(CPU *cpu, const char *operand);

/**
 * @brief Gestion de l'adressage indirect par registre.
 *
 * @param cpu Pointeur vers le CPU.
 * @param operand Opérande à analyser.
 * @return void* Pointeur vers la valeur indirecte.
 */
void *register_indirect_addressing(CPU *cpu, const char *operand);

/**
 * @brief Effectue une copie d'une valeur (src -> dest).
 *
 * @param cpu Pointeur vers le CPU.
 * @param src Pointeur vers la source.
 * @param dest Pointeur vers la destination.
 */
void handle_MOV(CPU *cpu, void *src, void *dest);

/**
 * @brief Résout un opérande en fonction de son mode d'adressage.
 *
 * @param cpu Pointeur vers le CPU.
 * @param operand Opérande à résoudre.
 * @return void* Pointeur vers la donnée résolue.
 */
void *resolve_addressing(CPU *cpu, const char *operand);

/**
 * @brief Alloue un segment ES (Extended Segment).
 *
 * @param cpu Pointeur vers le CPU.
 * @return int Retourne 0 si succès, -1 si échec.
 */
int alloc_es_segment(CPU *cpu);

/**
 * @brief Libère le segment ES (Extended Segment).
 *
 * @param cpu Pointeur vers le CPU.
 * @return int Retourne 0 si succès, -1 si échec.
 */
int free_es_segment(CPU *cpu);
/**
 * @brief Empile une valeur dans la pile du CPU.
 *
 * Cette fonction ajoute une valeur à la pile du CPU en la stockant dans la zone mémoire
 * réservée à cet effet. Elle vérifie si la pile est pleine avant d'ajouter la valeur 
 * et renvoie un code d'erreur si la pile est pleine. La pile est gérée dans la mémoire du CPU 
 * et fait partie de la structure `CPU`.
 *
 * @param cpu Pointeur vers le CPU dans lequel la valeur doit être empilée.
 * @param value Valeur entière à empiler dans la pile du CPU.
 * @return int Retourne 0 si la valeur a été empilée avec succès, -1 si la pile est pleine.
 */
int push_value(CPU *cpu, int value);

/**
 * @brief Dépile une valeur de la pile du CPU.
 *
 * Cette fonction extrait une valeur de la pile du CPU et la place dans la variable `dest`.
 * Elle vérifie si la pile est vide avant de tenter de dépiler une valeur, et renvoie -1 en cas
 * de pile vide. La pile fait partie de la structure mémoire du CPU.
 *
 * @param cpu Pointeur vers le CPU à partir duquel la valeur doit être dépilée.
 * @param dest Pointeur vers une variable qui recevra la valeur dépilée.
 * @return int Retourne 0 si la valeur a été dépilée avec succès, -1 si la pile est vide.
 */
int pop_value(CPU *cpu, int *dest);
