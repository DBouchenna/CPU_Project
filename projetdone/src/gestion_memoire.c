#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/gestion_memoire.h"




// Fonction d'initialisation du gestionnaire de mémoire
MemoryHandler *memory_init(int size) {
    if (size <= 0) return NULL;  // Vérification de la taille valide

    // Allocation du gestionnaire de mémoire
    MemoryHandler *handler = (MemoryHandler *)malloc(sizeof(MemoryHandler));
    if (!handler) {
        printf("Erreur : Allocation du gestionnaire de memoire echouee.\n");
        return NULL;
    }

    // Allocation du tableau de mémoire
    handler->memory = (void **)calloc(size, sizeof(void *));
    if (!handler->memory) {
        printf("Erreur : Allocation du tableau memoire echouee.\n");
        free(handler);
        return NULL;
    }

    // Création du segment libre couvrant toute la mémoire
    handler->free_list = (Segment *)malloc(sizeof(Segment));
    if (!handler->free_list) {
        printf("Erreur : Allocation du segment libre echouee.\n");
        free(handler->memory);
        free(handler);
        return NULL;
    }

    // Initialisation du segment libre
    handler->free_list->start = 0;
    handler->free_list->size = size;
    handler->free_list->next = NULL;

    // Initialisation de la table de hachage des allocations (HashMap)
    handler->allocated = hashmap_create();  // Assurez-vous que `hashmap_create()` est définie
    if (!handler->allocated) {
        printf("Erreur : Allocation de la table de hachage echouee.\n");
        free(handler->free_list);
        free(handler->memory);
        free(handler);
        return NULL;
    }

    handler->total_size = size;

    printf("Memoire initialisee avec %d unites.\n", size);
    return handler;
}
Segment *find_free_segment(MemoryHandler *handler, int start, int size, Segment **prev) {
    if (!handler || size <= 0) return NULL;  // Vérification des paramètres

    Segment *curr = handler->free_list;
    Segment *prec = NULL;  // Stocker le segment précédent

    while (curr) {
        // Vérifier si le segment actuel contient `start` et a assez de place pour `size`
        if (curr->start <= start && (curr->start + curr->size) >= (start + size)) {
            
            if (prev) {
                *prev = prec;  // Stocker le segment précédent si `prev` est non NULL
            }
            return curr;  // Trouvé !
        }

        // Mettre à jour `prev` et passer au segment suivant
        prec = curr;
        curr = curr->next;
    }

    return NULL;  // Aucun segment libre trouvé
}


int create_segment(MemoryHandler *handler, const char *name, int start, int size) {
    if (!handler || !name || size <= 0 || start < 0) {
        fprintf(stderr, "create_segment: paramètres invalides.\n");
        return -1;
    }

    // Trouver un segment libre contenant [start, start+size)
    Segment *prev = NULL;
    Segment *seg = find_free_segment(handler, start, size, &prev);
    if (!seg) {
        fprintf(stderr, "create_segment: espace insuffisant pour [%d, %d].\n", start, size);
        return -1;
    }

    // Création du segment alloué
    Segment *new_segment = (Segment *)malloc(sizeof(Segment));
    if (!new_segment) {
        fprintf(stderr, "create_segment: échec d'allocation.\n");
        return -1;
    }
    new_segment->start = start;
    new_segment->size = size;
    new_segment->next = NULL;

    // Ajouter à la table de hachage
    if (hashmap_insert(handler->allocated, name, new_segment) != 0) {
        fprintf(stderr, "create_segment: échec d'insertion dans la HashMap.\n");
        free(new_segment);
        return -1;
    }

    // Mise à jour de free_list
    int old_start = seg->start;
    int old_size = seg->size;
    Segment *old_next = seg->next;

    if (old_start == start && old_size == size) {
        // Segment utilisé complètement -> Supprimer de free_list
        if (!prev) handler->free_list = seg->next;
        else prev->next = seg->next;
        free(seg);
    } else if (old_start == start) {
        // Début du segment libre utilisé -> Réduire sa taille
        seg->start += size;
        seg->size -= size;
    } else if (start + size == old_start + old_size) {
        // Fin du segment libre utilisé -> Réduire sa taille
        seg->size = start - old_start;
    } else {
        // Milieu du segment libre utilisé -> Couper en deux
        Segment *after = (Segment *)malloc(sizeof(Segment));
        if (!after) {
            fprintf(stderr, "create_segment: échec d'allocation pour after.\n");
            return -1;
        }
        after->start = start + size;
        after->size = (old_start + old_size) - (start + size);
        after->next = old_next;
        seg->size = start - old_start;
        seg->next = after;
    }

    return 0;
}

int remove_segment(MemoryHandler *handler, const char *name) {
    
    if (!handler || !name) {
        fprintf(stderr, "remove_segment: paramètres invalides.\n");
        return -1;
    }

    // 1) Récupérer le segment dans `allocated`
    Segment *seg = (Segment *)hashmap_get(handler->allocated, name);
    if (!seg) {
        fprintf(stderr, "remove_segment: segment '%s' introuvable.\n", name);
        return -1;
    }

    // 2) Supprimer l'entrée dans `allocated`
    if (hashmap_remove(handler->allocated, name) != 0) {
        fprintf(stderr, "remove_segment: échec suppression '%s'.\n", name);
        return -1;
    }

    // 3) Réinsérer `seg` dans `free_list` (ordre croissant)
    Segment *prev = NULL;
    Segment *curr = handler->free_list;
    
    // Trouver la bonne position d’insertion (on insère avant le premier dont start >= seg->start)
    while (curr && curr->start < seg->start) {
        prev = curr;
        curr = curr->next;
    }
    
    // Insérer `seg` dans `free_list`
    seg->next = curr;
    if (!prev) {
        handler->free_list = seg;  // Insérer en tête
    } else {
        prev->next = seg;
    }

    // 4) Fusion avec le segment suivant (curr)
    if (curr && (seg->start + seg->size == curr->start)) {
        seg->size += curr->size;
        seg->next = curr->next;
        free(curr);
    }

    // 5) Fusion avec le segment précédent (prev)
    if (prev && (prev->start + prev->size == seg->start)) {
        prev->size += seg->size;
        prev->next = seg->next;
        free(seg);
    }

    return 0;
}
void destroy_memory_handler(MemoryHandler* m) {
    if (m == NULL) return;

    // Libérer la hashmap contenant les segments alloués.
    if (m->allocated != NULL) {
        hashmap_destroy(m->allocated);
    }

    // Libérer la liste chaînée des segments libres.
    Segment* courant = m->free_list;
    Segment* tmp = NULL;
    while (courant) {
        tmp = courant->next;
        free(courant);
        courant = tmp;
    }
    // free(courant); // Non nécessaire, courant vaut NULL ici.

    // Libérer chaque élément du tableau de mémoire.
    if (m->memory != NULL) {
        for (int i = 0; i < m->total_size; i++) {
            if (m->memory[i] != NULL) {
                free(m->memory[i]);
            }
        }
        free(m->memory);
    }

    // Enfin, libérer le gestionnaire de mémoire.
    free(m);
}

#include <limits.h>


/**
 * find_free_address_strategy :
 *   Parcourt handler->free_list pour trouver un segment libre d'au moins `size`
 *   unités selon la stratégie `strategy` :
 *     0 = First Fit, 1 = Best Fit, 2 = Worst Fit.
 *   Retourne l'adresse de début du bloc à allouer, ou -1 si aucun segment ne convient.
 */
int find_free_address_strategy(MemoryHandler *handler, int size, int strategy) {
    if (!handler || size <= 0) {
        return -1;
    }

    Segment *curr = handler->free_list;
    int best_start = -1;

    switch (strategy) {
        case 0:  // First Fit
            for (curr = handler->free_list; curr; curr = curr->next) {
                if (curr->size >= size) {
                    return curr->start;
                }
            }
            return -1;

        case 1: {  // Best Fit
            int best_diff = INT_MAX;
            for (curr = handler->free_list; curr; curr = curr->next) {
                if (curr->size >= size) {
                    int diff = curr->size - size;
                    if (diff < best_diff) {
                        best_diff = diff;
                        best_start = curr->start;
                        if (best_diff == 0) break;  // aucun gaspillage possible
                    }
                }
            }
            return best_start;
        }

        case 2: {  // Worst Fit
            int max_size = -1;
            for (curr = handler->free_list; curr; curr = curr->next) {
                if (curr->size >= size && curr->size > max_size) {
                    max_size = curr->size;
                    best_start = curr->start;
                }
            }
            return best_start;
        }

        default:
            // Stratégie non reconnue
            return -1;
    }
}
// Prototype de la fonction à tester

