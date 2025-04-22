#ifndef CODESEGMENT_H
#define CODESEGMENT_H
#include "dataSegment.h"

/**
 * Supprime les espaces, tabulations, retours à la ligne et retours chariot
 * en début et en fin de la chaîne.
 * @param str La chaîne à nettoyer.
 * @return La chaîne nettoyée sans espaces en début et en fin.
 */
char *trim(char *str);

/**
 * Parcourt la table `values` et remplace dans la chaîne *str* toutes les occurrences
 * des clés par leur valeur entière (convertie en chaîne de caractères).
 * @param str La chaîne dans laquelle les remplacements doivent être effectués.
 * @param values La table de hachage contenant les clés et valeurs à remplacer.
 * @return 1 si au moins un remplacement a été effectué, 0 sinon.
 */
int search_and_replace(char **str, HashMap *values);

/**
 * Résout les constantes dans une séquence d'instructions. Remplace les opérandes
 * dans les instructions à l'aide des tables `labels` et `memory_locations`.
 * - Si l'instruction a un seul opérande, elle est remplacée à partir de `labels`.
 * - Si l'instruction a deux opérandes, elles sont remplacées à partir de `memory_locations`.
 * @param result L'objet contenant les instructions à traiter.
 * @return 0 en cas de succès, -1 en cas d’erreur.
 */
int resolve_constants(ParserResult *result);

/**
 * Alloue un segment mémoire "CS" (Code Segment) dans le CPU pour stocker les instructions de code.
 * Cette fonction initialise également le registre IP (Instruction Pointer) à 0.
 * @param cpu Le CPU dans lequel le segment de code doit être alloué.
 * @param code_instructions Les instructions à stocker dans le segment.
 * @param code_count Le nombre d'instructions à allouer dans le segment de code.
 */
void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count);

/**
 * Gère l'exécution d'une instruction sur le CPU.
 * Résout les opérandes et exécute l'instruction (MOV, ADD, CMP, JMP, etc.).
 * @param cpu Le CPU sur lequel l'instruction sera exécutée.
 * @param instr L'instruction à exécuter.
 * @param src L'adresse de l'opérande source (peut être NULL).
 * @param dest L'adresse de l'opérande destination (peut être NULL).
 * @return 0 en cas de succès, -1 en cas d’erreur.
 */
int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest);

/**
 * Exécute une instruction sur le CPU.
 * Résout les opérandes source et destination, puis appelle `handle_instruction`.
 * @param cpu Le CPU sur lequel l'instruction sera exécutée.
 * @param instr L'instruction à exécuter.
 * @return 0 en cas de succès, -1 en cas d’erreur.
 */
int execute_instruction(CPU *cpu, Instruction *instr);

/**
 * Récupère la prochaine instruction à exécuter dans le Code Segment.
 * Incrémente le registre IP après la récupération de l'instruction.
 * @param cpu Le CPU contenant le registre IP et le Code Segment.
 * @return L'instruction suivante à exécuter, ou NULL si aucune instruction n'est disponible.
 */
Instruction* fetch_next_instruction(CPU *cpu);

/**
 * Affiche l'état actuel du segment de données du CPU (DS).
 * Affiche les adresses et les valeurs des données stockées dans le segment.
 * @param cpu Le CPU dont l'état du segment DS doit être affiché.
 */
void print_data_segment(CPU *cpu);

/**
 * Affiche l'état actuel des registres du CPU.
 * Affiche les valeurs des registres (AX, BX, CX, DX, IP, ZF, SF, ES).
 * @param cpu Le CPU dont les registres doivent être affichés.
 */
void print_registers(CPU *cpu);

/**
 * Exécute le programme stocké dans le Code Segment (CS).
 * Affiche l'état du CPU avant et après l'exécution des instructions.
 * L'exécution est pas-à-pas, et l'utilisateur peut intervenir pour arrêter l'exécution.
 * @param cpu Le CPU dans lequel le programme doit être exécuté.
 * @return 0 en cas de succès, -1 en cas d’erreur.
 */
int run_program(CPU *cpu);

#endif /* CODESEGMENT_H */
