#ifndef TH_GENERIQUE_H
#define TH_GENERIQUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 128  // Taille de la table de hachage

// =============================
// STRUCTURE : HashEntry
// =============================

/**
 * @brief Représente une entrée dans la table de hachage.
 * 
 * Chaque entrée de la table de hachage contient une clé sous forme de chaîne de caractères
 * et une valeur associée à cette clé. La valeur peut être de n'importe quel type, car elle est 
 * stockée sous forme de pointeur `void*`.
 */
typedef struct hashentry {
    char *key;        /**< Clé sous forme de chaîne de caractères */
    void *value;      /**< Valeur associée à la clé */
} HashEntry;

// =============================
// STRUCTURE : HashMap
// =============================

/**
 * @brief Représente une table de hachage.
 * 
 * Cette structure contient une table d'entrées de hachage. Elle utilise une taille fixe 
 * (`TABLE_SIZE`) pour déterminer le nombre de "buckets" ou compartiments dans la table.
 */
typedef struct hashmap {
    int size;         /**< Taille de la table de hachage */
    HashEntry *table; /**< Tableau d'entrées de hachage */
} HashMap;

// =============================
// FONCTIONS DE GESTION DE LA TABLE DE HACHAGE
// =============================

/**
 * @brief Fonction de hachage simple pour une chaîne de caractères.
 * 
 * Cette fonction calcule une valeur de hachage pour une chaîne donnée. Elle est utilisée 
 * pour déterminer l'emplacement de la clé dans la table de hachage.
 * 
 * @param str La chaîne de caractères à hacher.
 * @return unsigned long La valeur de hachage calculée pour la chaîne.
 */
unsigned long simple_hash(const char *str);

/**
 * @brief Crée et initialise une table de hachage.
 * 
 * Cette fonction crée une nouvelle table de hachage de taille fixe (`TABLE_SIZE`) et 
 * initialise toutes les entrées de la table à `NULL`.
 * 
 * @return HashMap* Pointeur vers la nouvelle table de hachage.
 */
HashMap *hashmap_create();

/**
 * @brief Insère un élément dans la table de hachage.
 * 
 * Cette fonction insère une nouvelle paire clé-valeur dans la table de hachage. Si une entrée 
 * avec la même clé existe déjà, elle est remplacée.
 * 
 * @param map Pointeur vers la table de hachage où l'élément doit être inséré.
 * @param key La clé sous forme de chaîne de caractères.
 * @param value Pointeur vers la valeur associée à la clé.
 * @return int Retourne 0 si l'insertion a réussi, sinon un code d'erreur.
 */
int hashmap_insert(HashMap *map, const char *key, void *value);

/**
 * @brief Récupère un élément de la table de hachage.
 * 
 * Cette fonction recherche la clé dans la table de hachage et retourne la valeur associée à 
 * cette clé. Si la clé n'existe pas, elle retourne `NULL`.
 * 
 * @param map Pointeur vers la table de hachage où rechercher l'élément.
 * @param key La clé de l'élément à récupérer.
 * @return void* Pointeur vers la valeur associée à la clé, ou `NULL` si la clé n'existe pas.
 */
void *hashmap_get(HashMap *map, const char *key);

/**
 * @brief Supprime un élément de la table de hachage.
 * 
 * Cette fonction supprime l'entrée associée à une clé donnée dans la table de hachage.
 * 
 * @param map Pointeur vers la table de hachage.
 * @param key La clé de l'élément à supprimer.
 * @return int Retourne 0 si la suppression a réussi, sinon un code d'erreur.
 */
int hashmap_remove(HashMap *map, const char *key);

/**
 * @brief Détruit la table de hachage et libère la mémoire associée.
 * 
 * Cette fonction détruit la table de hachage, en libérant toute la mémoire allouée pour 
 * les entrées et pour la table elle-même.
 * 
 * @param map Pointeur vers la table de hachage à détruire.
 */
void hashmap_destroy(HashMap *map);

#endif /* TH_GENERIQUE_H */