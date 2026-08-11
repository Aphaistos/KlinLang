# Klin

**Klin** est un langage de programmation système impératif bas niveau, conçu comme une alternative moderne, prévisible et minimaliste au C.

Il élimine la complexité inutile (pas de programmation orientée objet, pas de *garbage collector*, pas de gestionnaire d'invariants caché) au profit de trois piliers : **le contrôle total de la mémoire**, **une organisation claire par espaces de nommage** et **une syntaxe expressive**.

---

## Philosophie

* **Contrôle mémoire déterministe** : Pas de libération implicite ni de *garbage collection*. La gestion mémoire repose sur des arènes natives et des allocations explicites ($O(1)$).
* **Refus de OOP** : Pas de classes, d'héritage ni de méthodes liées (`this`). Le code est structuré en **structures d'état pures** et en **fonctions scopées**.
* **Zero Magic** : L'absence d'invariants système cachés repose entièrement sur la discipline du développeur. Le code fait exactement ce qui est écrit à l'écran.
* **Architecture en Bulles (*Space Bubbles*)** : Unification du namespace, du scope d'exécution et de la portée mémoire sous une même abstraction.

---

## Exemple Global

```klin
// Déclaration d'un namespace / d'une bulle
::mem::arena {

    [64] struct Arena {
        buffer:     *u8,
        capacity:   u64,
        pos:        u64,
        commit_pos: u64,
    }

    struct Temp {
        arena: *Arena,
        pos:   u64,
    }

    func push(arena_ptr: *Arena, size: u64, vol clear: bool) -> *u8 {
        val pos_aligned: u64 = (arena_ptr.pos + 7) & ~7;
        
        if (pos_aligned + size > arena_ptr.capacity) {
            -> null;
        }

        val out_ptr: *u8 = (((arena_ptr as usize) + pos_aligned) as *u8);
        arena_ptr.pos = pos_aligned + size;

        -> out_ptr;
    }

    func temp_begin(arena_ptr: *Arena) -> Temp {
        -> Temp {
            arena: arena_ptr,
            pos:   arena_ptr.pos,
        };
    }

    func temp_end(temp: Temp) {
        temp.arena.pos = temp.pos;
    }
}

// Point d'entrée principal
func main() -> i32 {
    var my_arena: ::mem::arena::Arena;
    
    // Traitement dans une bulle d'allocation temporaire
    val scratch: ::mem::arena::Temp = ::mem::arena::temp_begin(&my_arena);
    
    // ... allocations et calculs ...

    ::mem::arena::temp_end(scratch);
    -> 0;
}

```

---

## Aperçu de la Syntaxe

| Concept | Syntaxe Klin | Description |
| --- | --- | --- |
| **Immuabilité** | `val x: u64 = 42;` | Variable constante à la réassignation |
| **Mutabilité** | `var pos: usize = 0;` | Variable réassignable |
| **Retour** | `-> expr;` ou `->;` | Flèche explicite de retour de fonction |
| **Accès Volatile** | `vol *ptr` | Empêche l'optimisation du compilateur |
| **Cast mémoire** | `ptr as usize` | Conversion explicite de types bas niveau |
| **Sous-espace privé** | `::_ { ... }` | Symboles locaux au fichier courant |

---

## Structure de la Documentation (`/docs`)

```text
/docs
├── README.md                   <-- Guide d'architecture globale
├── 01-introduction/
│   ├── philosophy.md           <-- Principes bas niveau & refus de OOP
│   └── space-bubbles.md        <-- Concept des Bulles (Namespace + Arena + Scope)
├── 02-syntax-basics/
│   ├── variables-and-types.md  <-- val, var, types primitifs et casts (as)
│   ├── control-flow.md         <-- Control flow (if, while) et retour (->)
│   └── functions.md            <-- Signature des fonctions pures
├── 03-memory-and-pointers/
│   ├── pointers-and-casts.md   <-- Pointeurs (*T), conversions et accès volatile (vol)
│   └── native-arenas.md        <-- Modèle d'allocations par arènes natives
├── 04-architecture/
│   ├── namespaces.md           <-- Résolution de portée :: et bulles privées (::_)
│   └── structs.md              <-- Alignement [N] et disposition mémoire des données
└── 05-low-level/
    ├── inline-asm.md           <-- Bloc asm { ... }
    └── FFI-and-abi.md          <-- Interopérabilité C et modèle C ABI

```

---

## État du Projet

Le langage est actuellement au stade de **définition de la spécification et du frontend**. Le parseur C++ est en cours d'alignement avec la grammaire officielle dans le répertoire `/src`.