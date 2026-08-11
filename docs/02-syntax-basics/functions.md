# Déclaration de Fonctions et Liaisons Système

Dans **Klin**, les fonctions sont les unités fondamentales d'exécution. Elles sont pures par défaut, explicites dans leur typage et conçues pour minimiser le bruit syntaxique, aussi bien lors de l'écriture de code de haut niveau que lors de l'interface avec le matériel ou des liaisons système.

---

## 1. Syntaxe de Base (`func`) et Opérateur de Retour (`->`)

Les fonctions standards sont déclarées avec le mot-clé `func`. 

Pour alléger la syntaxe dans les blocs de code, Klin remplace le mot-clé traditionnel `return` par l'opérateur flèche `->`.

```klin
func add(a: i32, b: i32) -> i32 {
    -> a + b;
}

func calculate_offset(base: usize, index: usize, stride: usize) -> usize {
    val offset: usize = index * stride;
    -> base + offset;
}

```

* **Fonctions sans retour (`void`)** : Si la fonction ne renvoie aucune valeur, le type de retour est omis. L'instruction `->;` permet alors de quitter prématurément la fonction.

```klin
func log_error(code: u32) {
    if (code == 0) {
        ->; // Sortie anticipée
    }
    // Traitement de l'erreur...
}

```

---

## 2. Forme Courte pour Expressions Monolignes (`=>`)

Pour les fonctions simples dont le corps se résume à une unique expression ou instruction, Klin propose une syntaxe concis utilisant la double flèche `=>`.

Avec cette forme, les accolades `{}` et l'opérateur `->` ne sont pas nécessaires.

```klin
// Expression mathématique simple
func square(x: i32) -> i32 => x * x;

// Vérification de masque binaire
func is_aligned(addr: usize, align: usize) -> bool => (addr & (align - 1)) == 0;

// Instruction bas niveau monoligne
func hlt() => asm { "hlt" };

```

---

## 3. Fonctions Externes et Liaisons Bas Niveau (`_func`)

Pour interfacer le code Klin avec de l'assembleur, d'autres langages (C/ABI) ou des symboles exportés par le runtime/noyau, Klin utilise le préfixe `_func`.

Une fonction déclarée avec `_func` ne possède **pas de corps** (`{}`) dans le fichier source courant. Elle informe le compilateur qu'il doit résoudre le symbole lors de l'étape d'édition de liens (*linking*).

```klin
// Liaison avec une routine assembleur pour l'écriture sur un port I/O x86
_func __sys_outb(port: u16, val: u8);

// Liaison avec une fonction d'allocation bas niveau du noyau
_func __kernel_mmap(addr: usize, len: usize, flags: u32) -> *u8;

::drivers::serial {

    func send_byte(port: u16, data: u8) {
        // Appel direct de la fonction externe
        __sys_outb(port, data);
    }
}

```

---

## 4. Passage des Paramètres

1. **Passage par valeur** : Par défaut, tous les types primitifs (`i32`, `u64`, `bool`, etc.) et les structures sont passés par valeur.
2. **Passage par pointeur** : Pour modifier un argument ou éviter la copie de structures volumineuses, un pointeur brut `*T` ou un slice `[]T` doit être explicitement spécifié dans la signature.

```klin
struct Point {
    x: i32,
    y: i32,
}

// Reçoit un pointeur brut pour modifier la structure en place
func translate(p: *Point, dx: i32, dy: i32) {
    if (p == null) {
        ->;
    }
    p.x = p.x + dx;
    p.y = p.y + dy;
}

```

---

## 5. Synthèse de la Syntaxe des Fonctions

| Forme | Syntaxe | Usage principal |
| --- | --- | --- |
| **Bloc Standard** | `func name(p: T) -> R { -> expr; }` | Logique multi-lignes classique |
| **Forme Courte** | `func name(p: T) -> R => expr;` | Fonctions monolignes, getters, wrappers |
| **Fonction Externe** | `_func name(p: T) -> R;` | Symboles assembleur, ABI C, routines système |
| **Procedure (Void)** | `func name(p: T) { ... }` | Execution d'effets de bord sans valeur de retour |