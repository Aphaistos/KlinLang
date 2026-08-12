# Documentation du Langage Klin

**Klin** est un langage de programmation système conçu pour le développement *bare-metal*, la programmation de noyaux et le matériel embarqué. Il privilégie un contrôle explicite de la mémoire, une syntaxe épurée sans verbiage d'attributs et une absence de dépendances cachées.

---

## 📚 Sommaire de la Spécification

Cliquez sur les liens ci-dessous pour accéder directement à la documentation détaillée de chaque composant du langage :

### 01. Type System & Data Structures
* [**Structures, Enums et Dispositions Mémoire**](docs/01-types/structures-and-enums.md)  
  *Déclaration de `struct`, `enum`, compactage (`[] struct`), alignement sur mesure (`[N] struct`) et champs de bits (`bitfields`).*

### 02. Syntax & Control Flow
* [**Structures de Contrôle**](docs/02-syntax-basics/control-flow.md)  
  *Branchements (`if`/`else`), boucles d'intervalles OCaml-style (`for i = A -> B`), boucles conditionnelles (`while`), boucles infinies (`loop`) et filtrage (`match`).*

### 04. System Architecture
* [**Modules, Fichiers et Importations**](docs/04-architecture/modules-and-imports.md)  
  *Déclaration de modules (`mod`), liaison de fichiers (`->`), organisation en bulles d'espace (`::`) et directives d'importation (`imp`).*

### 05. Low-Level & Bare-Metal
* [**Assembleur en Ligne**](docs/05-low-level/inline-asm.md)  
  *Syntaxe des blocs `asm { "inst" : inputs : outputs }`, abstractions matérielles d'une ligne (`=>`) et gestion des effets de bord.*

---

## 🛠️ Principes de Conception

1. **Zéro Attribut Verbeux** : L'alignement (`[4096]`) et le compactage (`[]`) s'expriment sous forme de préfixes synthétiques directement sur la structure.
2. **Gestion Directe du Matériel** : Découpage au bit près (`u8 : 4`) et assembleur en ligne natif sans wrappers complexes.
3. **Compilation Rapide** : Importation de modules en une seule passe, prévenant les dépendances cycliques sans fichier d'en-tête (*headers*).
4. **Séparation Fichier / Espace de Nommage** : Les fichiers ne sont que des conteneurs de code organisés physiquement dans des modules logiques via l'instruction de lien `->`.