# Structures, Enums et Dispositions Mémoire

Dans **Klin**, les structures de données sont conçues pour offrir un contrôle exact sur l'organisation spatiale en mémoire. Pour répondre aux contraintes strictes du développement *bare-metal* (tables de descripteurs, pages de pagination, registres matériels), le langage intègre le compactage, l'alignement sur mesure et le découpage au niveau du bit directement dans sa syntaxe.

---

## 1. Structures Standard (`struct`)

Par défaut, les champs d'une structure sont ordonnés séquentiellement. Le compilateur peut insérer du rembourrage (*padding*) de manière à aligner chaque membre sur la frontière naturelle de son type.

```klin
struct Point {
    x: i32,
    y: i32,
}

struct DriverConfig {
    id: u32,
    enabled: bool,
}

```

---

## 2. Contrôle de la Disposition Mémoire

Plutôt que d'utiliser des pragmas ou des attributs verbeux, Klin contrôle l'alignement et le compactage au moyen de préfixes entre crochets appliqués directement à la déclaration de la structure.

### A. Structures Compactées (`[] struct`)

Un préfixe de crochets vides `[]` indique une structure **packed** (sans aucun rembourrage). Tous les champs sont contigus en mémoire. Cette forme est indispensable pour se conformer à des structures matérielles précises (ex: structures x86, en-têtes réseau).

```klin
// Registre pointeur de la GDT x86 (exactement 6 octets, sans padding)
[] struct GdtPtr {
    limit: u16,
    base:  u32,
}

```

### B. Structures Alignées (`[N] struct`)

Fournir une valeur numérique $N$ entre crochets force l'alignement de l'adresse de départ de la structure sur une frontière de $N$ octets. Utile pour manipuler des pages mémoire, des buffers DMA ou des lignes de cache.

```klin
// Table de pages alignée sur 4096 octets (4 KiB)
[4096] struct PageTable {
    entries: [u32; 1024],
}

```

---

## 3. Champs de Bits (*Bitfields*)

Klin permet de spécifier nativement la taille en bits d'un champ au moyen de la syntaxe `nom: type : nb_bits`. Cela permet de découper un octet ou un mot sans recourir à des masques ou décalages manuels.

```klin
// Entrée d'IDT x86 avec champs de bits explicites
[] struct IdtEntry {
    base_low:  u16,
    selector:  u16,
    zero:      u8,
    gate_type: u8 : 4,
    storage:   u8 : 1,
    dpl:       u8 : 2,
    present:   u8 : 1,
    base_high: u16,
}

```

---

## 4. Énumérations (`enum`)

Les énumérations définissent un ensemble de constantes nommées basées sur un type entier sous-jacent.

```klin
enum InterruptVector : u8 {
    Timer    = 0x20,
    Keyboard = 0x21,
    Syscall  = 0x80,
}

enum PageFlags : u32 {
    Present  = 1 << 0,
    Writable = 1 << 1,
    User     = 1 << 2,
}

```

---

## 5. Synthèse des Annotations de Disposition

| Syntaxe | Effet sur la disposition mémoire | Usage typique |
| --- | --- | --- |
| `struct T` | Alignement naturel par membre avec padding | Structures applicatives et données internes |
| `[] struct T` | Désactive le padding (100% contigu, *packed*) | En-têtes protocolaires, structures x86 (GDT/IDT) |
| `[N] struct T` | Enforce un alignement de départ sur $N$ octets | Tables de pagination, buffers DMA, alignement de cache |
| `member: T : B` | Restreint le champ à $B$ bits | Registres matériels, drapeaux de contrainte |