#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h> 

#include "../include/dataSegment.h"



int push_value(CPU *cpu, int value) {
    if (!cpu) return -1;

    // 1) Récupérer SP et segment SS
    int *sp = (int*)hashmap_get(cpu->context, "SP");
    Segment *ss = (Segment*)hashmap_get(cpu->memory_handler->allocated, "SS");
    if (!sp || !ss) return -1;

    // 2) Vérifier overflow : SP ne doit pas descendre sous ss->start
    if (*sp <= ss->start) {
        // pile pleine
        return -1;
    }

    // 3) Décrémenter SP
    (*sp)--;

    // 4) Allouer et stocker la valeur
    int *cell = malloc(sizeof(int));
    if (!cell) {
        // rollback SP
        (*sp)++;
        return -1;
    }
    *cell = value;
    cpu->memory_handler->memory[*sp] = cell;

    return 0;
}


int pop_value(CPU *cpu, int *dest) {
    if (!cpu || !dest) return -1;

    // 1) Récupérer SP et segment SS
    int *sp = (int*)hashmap_get(cpu->context, "SP");
    Segment *ss = (Segment*)hashmap_get(cpu->memory_handler->allocated, "SS");
    if (!sp || !ss) return -1;

    // 2) Underflow : SP ne doit pas dépasser ss->start + ss->size
    int stack_top = ss->start + ss->size;
    if (*sp >= stack_top) {
        // pile vide
        return -1;
    }

    // 3) Lire la valeur
    int *cell = (int*)cpu->memory_handler->memory[*sp];
    if (!cell) return -1;
    *dest = *cell;

    // 4) Libérer la case et incrémenter SP
    free(cell);
    cpu->memory_handler->memory[*sp] = NULL;
    (*sp)++;

    return 0;
}
