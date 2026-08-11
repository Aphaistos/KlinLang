# Structures, Disposition Mémoire et Absence d'Invariants

Dans **Klin**, une `struct` est une agrégation pure et explicite de données en mémoire. Le langage rejette le modèle orienté objet : il n'existe ni constructeurs, ni destructeurs, ni méthodes liées (`this`), ni héritage. 

L'agencement des octets est prévisible, transparent et entièrement sous le contrôle du développeur.

---

## 1. Philosophie : Absence d'Invariants Système

Dans les langages orientés objet ou à abstractions lourdes, les structures maintiennent souvent des **invariants système** invisibles (tables de méthodes virtuelles `vtable`, pointeurs d'invalidation, compteurs de références ou méthodes d'initialisation implicites).

Klin applique le principe du **Zero Magic** :
* **Aucun code caché** : L'instanciation d'une structure ne déclenche aucun appel de fonction implicite.
* **Transparence mémoire** : La taille de la structure correspond exactement à la somme de ses membres et de ses contraintes d'alignement/rembourrage (*padding*).
* **Discipline du développeur** : La validité des données n'est pas garantie par des invariants système ou de la métaprogrammation, mais par des fonctions pures explicites organisées dans des espaces de nommage (`::namespace`).

---

## 2. Syntaxe de Déclaration

Par défaut, les membres d'une structure sont alignés selon les règles naturelles de l'architecture cible pour optimiser la vitesse d'accès par le processeur.

```klin
struct Point {
    x: i32,
    y: i32,
}
```

## 3. Contrôle de Disposition Mémoire (Layout)

Klin permet de modifier la disposition mémoire des structures au moyen d'annotations explicites sous forme de crochets placés en préfixe de la déclaration :

### A. Structures Alignées (`[N]`)

L'annotation `[N]` force l'adresse de départ de la structure à être alignée sur une frontière de `N` octets. Cela est crucial pour les structures interactives avec le matériel, la mémoire virtuelle ou le cache processeur (ex: 64 octets pour une ligne de cache, 4096 octets pour une page mémoire).

```klin
// Alignement strict sur une frontière de page mémoire (4096 octets)
[4096] struct PageTable {
    entries: [u32; 1024],
}
```

### B. Structures Compactées (`[]`)
L'annotation `[]` (packed) supprime tout le rembourrage (padding) inséré par le compilateur entre les membres. La structure occupe le strict minimum d'octets requis, ce qui est indispensable pour mapper des registres matériels, des tables d'interruptions ou des en-têtes réseau.

```klin
// Aucune perte d'octets entre limit (16 bits) et base (32 bits)
[] struct GdtPtr {
    limit: u16,
    base:  u32,
}
```

## 4. Champs de Bits Nativement Intégrés (Bitfields)
Klin permet de spécifier la largeur exacte en bits d'un membre directement après son type à l'aide de la syntaxe `: n_bits`.

Le compilateur génère automatiquement les masques et décalages d'instructions (*shift/and/or*) lors des accès, évitant les opérations binaires manuelles répétitives.

```klin
[] struct IdtEntry {
    base_low:  u16,
    selector:  u16,
    zero:      u8,
    
    // Champs de bits
    gate_type: u8 : 4,  // 4 bits : type de porte d'interruption
    storage:   u8 : 1,  // 1 bit  : segment de stockage
    dpl:       u8 : 2,  // 2 bits : niveau de privilège (Ring 0-3)
    present:   u8 : 1,  // 1 bit  : bit de présence
    
    base_high: u16,
}
```

## 5. Exemple, Cas d'Étude Canonique : La Structure `Arena`

La structure `Arena` illustre parfaitement l'utilisation d'une structure alignée pour la gestion mémoire bas niveau dans Klin.

Aligner l'arène sur **64 octets** (la taille standard d'une ligne de cache) empêche le phénomène de *false sharing* en contexte multithread et garantit que les champs fréquemment modifiés (`pos`, `commit_pos`) résident sur la même ligne de cache sans déborder.

```klin
::mem::arena {

    // Alignement sur 64 octets (Cache Line Boundary)
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

    // Fonction pure modifiant l'état de la structure transmise par pointeur
    func push(arena_ptr: *Arena, size: u64, val align: u64) -> *u8 {
        val align_mask: u64  = align - 1;
        val pos_aligned: u64 = (arena_ptr.pos + align_mask) & ~align_mask;
        
        if (pos_aligned + size > arena_ptr.capacity) {
            -> null;
        }

        val out_ptr: *u8 = (((arena_ptr.buffer as usize) + pos_aligned) as *u8);
        arena_ptr.pos = pos_aligned + size;

        -> out_ptr;
    }
}
```