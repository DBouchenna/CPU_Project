#ifndef GESTION_MEMOIRE_H
#define GESTION_MEMOIRE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "perser.h"

 /* @brief Structure représentant un segment de mémoire.
 * 
 * Cette structure décrit un segment de mémoire, avec des informations sur son 
 * emplacement (start) et sa taille, ainsi qu'un pointeur vers le segment suivant.
 */
typedef struct segment {
    int start;              /**< Début du segment */
    int size;               /**< Taille du segment */
    struct segment *next;   /**< Pointeur vers le segment suivant */
} Segment;

/**
 * @brief Structure principale du gestionnaire de mémoire.
 * 
 * Cette structure gère l'allocation et la libération de mémoire, en maintenant un 
 * tableau de mémoire allouée, une liste de segments libres, et une table de hachage 
 * des segments alloués.
 */
typedef struct memoryHandler {
    void **memory;      /**< Tableau de mémoire allouée */
    int total_size;     /**< Taille totale de la mémoire */
    Segment *free_list; /**< Liste des segments libres */
    HashMap *allocated; /**< Table de hachage des segments alloués */
} MemoryHandler;

/**
 * @brief Initialise la mémoire du gestionnaire.
 * 
 * Cette fonction alloue un gestionnaire de mémoire, prépare la mémoire, et crée 
 * une liste de segments libres. Elle retourne un pointeur vers le gestionnaire de mémoire 
 * ou NULL en cas d'échec.
 * 
 * @param size Taille de la mémoire à initialiser.
 * @return MemoryHandler* Pointeur vers le gestionnaire de mémoire ou NULL en cas d'erreur.
 */
MemoryHandler *memory_init(int size);

/**
 * @brief Trouve un segment libre correspondant aux critères spécifiés.
 * 
 * Cette fonction parcourt la liste des segments libres pour trouver un segment qui 
 * contient l'adresse de début `start` et a suffisamment d'espace pour `size` unités. 
 * Elle retourne le segment trouvé ou NULL si aucun segment libre ne convient.
 * 
 * @param handler Pointeur vers le gestionnaire de mémoire.
 * @param start Adresse de début recherchée dans la mémoire.
 * @param size Taille du segment recherché.
 * @param prev Pointeur vers un pointeur pour stocker le segment précédent dans la liste.
 * @return Segment* Pointeur vers le segment libre trouvé, ou NULL si aucun segment ne convient.
 */
Segment *find_free_segment(MemoryHandler *handler, int start, int size, Segment **prev);

/**
 * @brief Crée un segment de mémoire alloué.
 * 
 * Cette fonction crée un nouveau segment de mémoire alloué dans la mémoire. Elle met 
 * à jour la table de hachage des allocations et modifie la liste des segments libres.
 * 
 * @param handler Pointeur vers le gestionnaire de mémoire.
 * @param name Nom du segment à allouer.
 * @param start Adresse de début du segment à allouer.
 * @param size Taille du segment à allouer.
 * @return int Retourne 0 si l'allocation a réussi, -1 en cas d'erreur.
 */
int create_segment(MemoryHandler *handler, const char *name, int start, int size);

/**
 * @brief Supprime un segment de mémoire alloué.
 * 
 * Cette fonction libère un segment alloué en le retirant de la table de hachage des allocations. 
 * Elle réinsère ensuite ce segment dans la liste des segments libres, avec des fusions possibles 
 * avec les segments adjacents pour éviter la fragmentation.
 * 
 * @param handler Pointeur vers le gestionnaire de mémoire.
 * @param name Nom du segment à libérer.
 * @return int Retourne 0 si la suppression a réussi, -1 en cas d'erreur.
 */
int remove_segment(MemoryHandler *handler, const char *name);

/**
 * @brief Libère toutes les ressources associées au gestionnaire de mémoire.
 * 
 * Cette fonction libère toutes les ressources allouées, y compris la mémoire allouée, 
 * la table de hachage des segments alloués et la liste des segments libres.
 * 
 * @param m Pointeur vers le gestionnaire de mémoire à libérer.
 */
void destroy_memory_handler(MemoryHandler* m);

/**
 * @brief Recherche un segment libre selon une stratégie de recherche.
 * 
 * Cette fonction parcourt la liste des segments libres pour trouver un segment qui 
 * peut accueillir un bloc de mémoire de taille `size`. La stratégie de recherche peut être :
 * - 0 : First Fit
 * - 1 : Best Fit
 * - 2 : Worst Fit
 * Elle retourne l'adresse de début du segment trouvé ou -1 si aucun segment ne convient.
 * 
 * @param handler Pointeur vers le gestionnaire de mémoire.
 * @param size Taille du bloc à allouer.
 * @param strategy Stratégie de recherche (0 = First Fit, 1 = Best Fit, 2 = Worst Fit).
 * @return int Adresse de début du segment trouvé ou -1 si aucun segment ne convient.
 */
int find_free_address_strategy(MemoryHandler *handler, int size, int strategy);
#endif /* GESTION_MEMOIRE_H */