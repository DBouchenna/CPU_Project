#ifndef PERSER_H
#define PERSER_H

#include "th_generique.h"

// =============================
// STRUCTURE : Instruction
// =============================

/**
 * @brief Représente une instruction, soit dans la section .DATA soit dans la section .CODE.
 * 
 * Cette structure contient le mnémonique de l'instruction et ses opérandes. Dans la section 
 * .DATA, elle représente une variable avec son nom et ses valeurs initiales, tandis que dans 
 * la section .CODE, elle représente une instruction avec ses opérandes.
 */
typedef struct Instruction {
    char *mnemonic;   /**< Le mnémonique de l'instruction (ex: MOV, ADD ou nom de variable pour .DATA) */
    char *operand1;   /**< Premier opérande de l'instruction (ou type pour .DATA) */
    char *operand2;   /**< Second opérande de l'instruction (ou valeurs initiales pour .DATA) */
} Instruction;

// =============================
// STRUCTURE : ParserResult
// =============================

/**
 * @brief Contient les résultats du parsing d'un fichier source.
 * 
 * Cette structure contient les instructions de deux sections : .DATA et .CODE. Elle garde 
 * également les informations de mappage entre les labels et les indices dans les instructions 
 * .CODE ainsi que les adresses mémoire des variables définies dans .DATA.
 */
typedef struct {
    Instruction **data_instructions;   /**< Tableau d'instructions pour la section .DATA */
    int data_count;                    /**< Nombre d'instructions dans la section .DATA */

    Instruction **code_instructions;   /**< Tableau d'instructions pour la section .CODE */
    int code_count;                    /**< Nombre d'instructions dans la section .CODE */

    HashMap *labels;                   /**< Map associant un label à son indice dans les instructions .CODE */
    HashMap *memory_locations;         /**< Map associant le nom d'une variable à son adresse mémoire */
} ParserResult;

// =============================
// FONCTIONS EXPORTÉES
// =============================

/**
 * @brief Analyse une ligne de la section .DATA.
 * 
 * Cette fonction analyse une ligne représentant une instruction dans la section .DATA et retourne 
 * une structure `Instruction` avec les informations pertinentes (nom, opérandes). Elle met également 
 * à jour la table de hachage des emplacements mémoire des variables.
 * 
 * @param line La ligne à analyser, représentant une instruction dans .DATA.
 * @param memory_locations La table de hachage des emplacements mémoire.
 * @return Instruction* Pointeur vers une structure `Instruction` représentant l'instruction analysée.
 */
Instruction *parse_data_instruction(const char *line, HashMap *memory_locations);

/**
 * @brief Analyse une ligne de la section .CODE.
 * 
 * Cette fonction analyse une ligne représentant une instruction dans la section .CODE et retourne 
 * une structure `Instruction` avec les informations pertinentes (mnémonique, opérandes). Elle met 
 * également à jour la table de hachage des labels.
 * 
 * @param line La ligne à analyser, représentant une instruction dans .CODE.
 * @param labels La table de hachage des labels.
 * @param code_count Le compteur de lignes de code (utilisé pour l'indice de l'instruction).
 * @return Instruction* Pointeur vers une structure `Instruction` représentant l'instruction analysée.
 */
Instruction *parse_code_instruction(const char *line, HashMap *labels, int code_count);

/**
 * @brief Retourne la valeur actuelle du compteur global (adresse mémoire).
 * 
 * Cette fonction retourne l'adresse mémoire actuelle à partir du compteur global, qui est mis à jour 
 * lors du parsing des sections .DATA et .CODE.
 * 
 * @return int La valeur actuelle du compteur (adresse mémoire).
 */
int get_compteur_value();

/**
 * @brief Affiche une liste d'instructions dans la console.
 * 
 * Cette fonction permet d'afficher les instructions de type `Instruction` dans la console, 
 * en format lisible pour l'utilisateur.
 * 
 * @param tableau Tableau d'instructions à afficher.
 * @param taille Taille du tableau.
 */
void afficherInstructions(Instruction *tableau[], int taille);

/**
 * @brief Parse un fichier assembleur et retourne un résultat de parsing.
 * 
 * Cette fonction parse un fichier assembleur et retourne une structure `ParserResult` contenant 
 * les instructions .DATA, .CODE, ainsi que les labels et les adresses mémoire des variables.
 * 
 * @param filename Le nom du fichier à analyser.
 * @return ParserResult* Pointeur vers la structure `ParserResult` contenant les résultats du parsing.
 */
ParserResult *parse(const char *filename);

/**
 * @brief Libère la mémoire associée à un résultat de parsing.
 * 
 * Cette fonction libère toute la mémoire allouée pour les instructions et les tables de hachage 
 * dans la structure `ParserResult`.
 * 
 * @param result Pointeur vers le `ParserResult` à libérer.
 */
void free_parser_result(ParserResult *result);

#endif /* PERSER_H */