# Mini-Processeur - Projet d'Architecture des Ordinateurs

## 📌 Présentation

Ce projet a pour objectif de concevoir une **version simplifiée d’un processeur**. Un processeur (CPU - Central Processing Unit) est le cœur de l’ordinateur, responsable de l'exécution des instructions d’un programme, de la gestion des registres, de la mémoire, et du contrôle du flux d’exécution.

L’implémentation de ce projet se concentre sur :
- Un ensemble réduit de **registres** (AX, BX, CX, DX, IP, SP, FLAGS),
- Une **unité de contrôle** simplifiée,
- Une **ALU (unité arithmétique et logique)** basique,
- L’**accès à la mémoire** à travers différents modes d’adressage,
- Et la **gestion du cycle d’exécution d’instruction** : Fetch – Decode – Execute.

## 🧠 Objectifs pédagogiques

- Comprendre le rôle et le fonctionnement d’un processeur.
- Manipuler les registres et la mémoire.
- Implémenter une machine simplifiée exécutant des instructions.
- Approfondir la notion de cycle d’instruction.
- Se familiariser avec la simulation bas-niveau d’un système informatique.

## 🛠️ Technologies utilisées

- Langage : C 
- Environnement : Linux / Windows
- Éditeur conseillé : VS Code

## 📁 Contenu du dépôt

- `src/` : fichiers sources du projet (implémentation du processeur, de l’ALU, etc.)
- `include/` : fichiers d’en-tête (si applicable)

## ⚙️ Fonctionnalités attendues

- Cycle d’instruction de base (fetch, decode, execute)
- Gestion des registres et de la pile (stack)
- Instructions de base : MOV, ADD, SUB, JMP, CMP, etc.
- Mode d’adressage immédiat et mémoire directe
- Interface ligne de commande pour charger un programme et exécuter les instructions pas à pas

## 🧪 Tests

Un programme de test est fournis pour valider :
- L’exactitude des calculs de l’ALU
- La bonne exécution des instructions
- Le comportement attendu de la mémoire et des registres




