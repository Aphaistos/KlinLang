# Documentation Officielle du Langage Klin

Bienvenue dans la documentation de **Klin**, un langage de programmation système conçu pour le développement *bare-metal*, la programmation système bas niveau et le contrôle explicite des ressources matérielles.

---

## Sommaire de la Spécification

### 01. Type System & Layout (`01-types/`)
* [Structures, Enums & Dispositions Mémoire](01-types/structures-and-enums.md)

### 02. Syntax Basics (`02-syntax-basics/`)
* [Structures de Contrôle de Flux](02-syntax-basics/control-flow.md)
* [Fonctions & Abstractions](02-syntax-basics/functions.md)

### 03. Memory & Low-Level Pointers (`03-memory-and-pointers/`)
* [Pointeurs Bruts & Casts Explictes](03-memory-and-pointers/pointers-and-casts.md)
* [Slices & Séquences Contiguës](03-memory-and-pointers/slices-and-arrays.md)

### 04. System Architecture (`04-architecture/`)
* [Modules, Fichiers & Importations](04-architecture/modules-and-imports.md)
* [Space Bubbles & Hiérarchie de Visibilité](04-architecture/spacebubbles.md)
* [Déclaration & Méthodes de Structures](04-architecture/structs.md)

### 05. Low-Level Hardware Access (`05-low-level/`)
* [Assembleur en Ligne (`asm { ... }`)](05-low-level/inline-asm.md)

---

## Vue d'Ensemble des Spécificités

| Domaine | Caractéristique principale |
| :--- | :--- |
| **Mémoire & Alignement** | Syntaxe concise `[] struct` (packed) et `[N] struct` (alignement $N$ octets) |
| **Modules & Code** | Séparation des conteneurs de fichiers (`-> "file.kln"`) et des bulles logiques |
| **Bare-Metal** | Intégration native des *bitfields* (`x: u8 : 4`) et de l'assembleur en ligne `asm` |

```