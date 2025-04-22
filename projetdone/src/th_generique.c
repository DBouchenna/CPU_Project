#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/th_generique.h"


#define TABLE_SIZE 128
#define TOMBSTONE (( void *) -1)


// Fonction de hachage simple : somme les valeurs ASCII des caractères

unsigned long simple_hash(const char *str) {
    unsigned long hash = 0;
    while (*str != '\0') {
        hash += (unsigned char)(*str);  
        str++;
    }
    return hash % TABLE_SIZE;
}


// Création d'une nouvelle HashMap
HashMap *hashmap_create() {
    HashMap *newHash = (HashMap *)malloc(sizeof(HashMap));
    if (!newHash) {
        printf("Erreur d'allocation mémoire pour HashMap\n");
        return NULL;
    }

    newHash->size = TABLE_SIZE;

    // Allocation du tableau de HashEntry
    newHash->table = (HashEntry *)malloc(sizeof(HashEntry) * TABLE_SIZE);
    if (!newHash->table) {
        printf("Erreur d'allocation mémoire pour la table\n");
        free(newHash);
        return NULL;
    }

    // Initialisation des entrées à NULL
    for (int i = 0; i < TABLE_SIZE; i++) {
        newHash->table[i].key = NULL;
        newHash->table[i].value = NULL;
    }

    return newHash;
}

int hashmap_insert(HashMap *map, const char *key, void *value) {
   
    if (!map || !key) return -1;  // Vérification des paramètres

    unsigned long index = simple_hash(key);
    
    int tombstone_index = -1;  // Index du premier TOMBSTONE trouvé

    // Recherche d'un emplacement libre ou d'un TOMBSTONE
    while (map->table[index].key != NULL) {
        if (map->table[index].key == TOMBSTONE && tombstone_index == -1) {
            tombstone_index = index;  // Mémorise le premier TOMBSTONE trouvé
        }
        if (strcmp(map->table[index].key, key) == 0) {
            // Mise à jour de la valeur si la clé existe déjà
            map->table[index].value = value;
            return 0;
        }
        index = (index + 1) % map->size;  // Probing linéaire
    }

    // Si on a trouvé un TOMBSTONE, on l’utilise pour insérer l’élément
    if (tombstone_index != -1) {
        index = tombstone_index;
    }

    // Insérer la nouvelle clé et la valeur
    map->table[index].key = strdup(key);
    
    // Copier la clé pour éviter une perte de mémoire
    if (!map->table[index].key) return -1; // Vérification de `strdup`
    map->table[index].value = value;

    return 0;  // Succès
}

void *hashmap_get(HashMap *map, const char *key) {
    
    if (!map || !key) return NULL;  // Vérification des paramètres

    unsigned long index = simple_hash(key);
    
    while (map->table[index].key != NULL) {
       
        
        // Vérifier que la clé n'est PAS un TOMBSTONE avant d'utiliser strcmp()
        if (map->table[index].key != TOMBSTONE && strcmp(map->table[index].key, key) == 0) {
           
            return map->table[index].value;  // Clé trouvée, renvoyer la valeur associée
        }
        index = (index + 1) % map->size;  // Probing linéaire
    }

    return NULL;  // Clé non trouvée
}
int hashmap_remove(HashMap *map, const char *key) {
    if (!map || !key) return -1;  // Vérification des paramètres    
    unsigned long index = simple_hash(key);
    while (map->table[index].key != NULL) {
        if (map->table[index].key != TOMBSTONE && strcmp(map->table[index].key, key) == 0) {
     
     free(map->table[index].key);
     
     map->table[index].key = TOMBSTONE;
     map->table[index].value = NULL;
     return 0;
    }
    }
    return -1;
}


void hashmap_destroy(HashMap *map) {
    if (!map) return;  // Vérification du pointeur

    for (int i = 0; i < map->size; i++) {
        if (map->table[i].key != NULL && map->table[i].key != TOMBSTONE) {
            free(map->table[i].key);  // Libérer la clé
            // Si les valeurs ont été allouées dynamiquement et doivent être libérées, ajoutez : free(map->table[i].value);
            map->table[i].value = NULL;  // Optionnel : remise à zéro du pointeur (non indispensable ici)
        }
    }
    free(map->table);  // Libérer le tableau de la hashmap après avoir parcouru toutes les entrées
    free(map);         // Libérer la structure de la hashmap
}


