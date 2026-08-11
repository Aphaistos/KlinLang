# Tableaux Statiques et Slices (`[]T`)

Dans **Klin**, la gestion de séquences de données contiguës repose sur deux concepts distincts mais parfaitement complémentaires :
1. **Les tableaux de taille fixe (`[T; N]`)** : Alloués statiquement sur la pile (*stack*) ou dans la section de données.
2. **Les Slices (`[]T`)** : Des vues mémoire dynamiques et non-propriétaires définies par un pointeur et une longueur.

---

## 1. Tableaux de Taille Fixe (`[T; N]`)

Un tableau est un bloc contigu de `N` éléments de type `T`. Sa taille est connue à la compilation et fait partie intégrante de son type.

```klin
// Tableau de 256 octets initialisé à zéro sur la pile
var stack_buffer: [u8; 256] = [0; 256];

// Tableau de constantes
const VECTORS: [i32; 3] = [1, 0, -1];

```

* **Passage en mémoire** : La taille étant fixe, copier un tableau copie la totalité de ses éléments. Pour éviter cette surcharge, les tableaux sont généralement découpés sous forme de slice lors du passage aux fonctions.

---

## 2. Slices (`[]T`) : Vues Mémoire Dynamiques

Un **slice** (`[]T`) est une vue légère, de taille dynamique, sur une région mémoire contiguë existante.

### Structure Interne d'un Slice

Sous le capot, un `[]T` est une structure compacte contenant exactement deux champs (taille d'un mot processeur chacun) :

* `ptr: *T` — Le pointeur vers le premier élément de la séquence.
* `len: usize` — Le nombre d'éléments accessibles dans la vue.

Un slice n'alloue et ne libère jamais de mémoire lui-même : il emprunte la mémoire sous-jacente.

---

## 3. Création et Conversion de Slices

Klin offre une syntaxe unifiée avec l'opérateur de plage `[debut..fin]` pour extraire un slice à partir d'un tableau ou convertir un pointeur brut.

### A. Extrait d'un Tableau Statique

```klin
var stack_buffer: [u8; 256] = [0; 256];

// Crée une vue sur les éléments de l'index 10 à 19 inclus (longueur = 10)
var view: []u8 = stack_buffer[10..20];

```

### B. Conversion de Pointeur Brut en Slice Sécurisé

C'est un motif essentiel pour la programmation système et le développement de pilotes matériels. Un pointeur brut `*T` associé à une plage `[0..N]` se transforme instantanément en un slice manipulable en toute sécurité :

```klin
::drivers::vga {

    // Conversion de l'adresse physique VGA text mode (80x25 caractères u16)
    func get_framebuffer() -> []u16 {
        var raw_ptr: *u16 = 0xB8000 as *u16;
        -> raw_ptr[0..2000];
    }

    func clear_screen(color: u8) {
        var fb: []u16 = get_framebuffer();
        var i: usize = 0;
        
        while (i < fb.len) {
            fb[i] = (color as u16) << 8;
            i = i + 1;
        }
    }
}

```

---

## 4. Synthèse des Différences

| Feature | Tableau (`[T; N]`) | Slice (`[]T`) |
| --- | --- | --- |
| **Taille** | Fixe à la compilation | Dynamique à l'exécution |
| **Empreinte Mémoire** | `sizeof(T) * N` | `sizeof(*T) + sizeof(usize)` (16 octets sur 64-bit) |
| **Propriété** | Contient la donnée | Vue non-propriétaire sur de la mémoire existante |
| **Usage Typique** | Buffers locaux, tables statiques | Arguments de fonctions, manipulation de fenêtres mémoire |