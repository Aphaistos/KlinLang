# Pointeurs Bruts, Casts Explicites et Accès Volatile (`vol`)

Dans **Klin**, le contrôle de la mémoire est direct, explicite et sans abstraction cachée. Le langage ne possède ni référence gérée, ni ramasse-miettes (*garbage collector*), ni vérificateur d'emprunt (*borrow checker*). 

La manipulation directe de la mémoire matérielle s'appuie sur des pointeurs bruts `*T`, la conversion explicite avec l'opérateur `as`, et le contrôle fin des accès mémoire via le mot-clé `vol`.

---

## 1. Pointeurs Bruts (`*T`)

Un pointeur brut `*T` stocke l'adresse mémoire directe d'une valeur de type `T`.

* **Pointeur Nul** : La valeur `null` représente un pointeur invalide (adresse `0x0`).
* **Pointeur Générique (`*u8`)** : Utilisé par convention pour la manipulation de blocs d'octets brut (ex: buffers d'arène).
* **Déférencement (`*ptr`)** : L'opérateur `*` permet de lire ou d'écrire la valeur pointée par une adresse.

```klin
func scale_value(val_ptr: *i32, factor: i32) {
    if (val_ptr == null) {
        ->;
    }
    *val_ptr = (*val_ptr) * factor;
}

```

---

## 2. Conversions Explicites de Types (`as`)

Klin **interdit tout cast implicite** entre types numériques ou pointeurs, même s'il n'y a aucune perte de précision. Chaque conversion de type doit être formulée explicitement avec le mot-clé `as`.

### A. Conversions d'Adresses et de Integers

Pour effectuer de l'arithmétique de pointeurs ou convertir une adresse physique MMIO en pointeur utilisable, la valeur doit être convertie vers le type entier `usize` (taille du mot processeur) avant réallocation :

```klin
const MMIO_REG: *u32 = 0xF0000000 as *u32;

func offset_pointer(ptr: *u8, bytes: usize) -> *u8 {
    val addr: usize = ptr as usize;
    -> (addr + bytes) as *u8;
}

```

### B. Troncature et Extension

* **Conversion d'extension** : N'altère pas les données mais requiert l'opérateur `as` (ex: `u8` vers `u64`).
* **Conversion de troncature** : Conserve uniquement les bits de poids faible de la valeur d'origine.

---

## 3. Accès Mémoire Volatile (`vol`)

Lors du développement de noyaux (*kernel*), de pilotes matériels (*drivers*) ou d'I/O mappées en mémoire (MMIO), le compilateur pourrait tenter d'optimiser, de réordonner ou de supprimer des lectures/écritures mémoire qu'il juge redondantes.

Le mot-clé `vol` résout ce problème en forçant une instruction de lecture ou d'écriture physique brute.

### Règle de Syntaxe

Placé **directement devant une opération d'accès mémoire** (`vol *ptr` ou `vol ptr[i]`), `vol` garantit que :

1. L'accès mémoire physique aura lieu exactement au moment où l'instruction apparaît.
2. L'opération ne sera ni supprimée par élimination de dead-code, ni réordonnée par rapport aux autres accès volatiles.

```klin
::hardware::timer {

    ::_ {
        const TIMER_CONTROL: *u32 = 0x40000000 as *u32;
        const TIMER_VALUE:   *u32 = 0x40000004 as *u32;
    }

    func init_timer(period: u32) {
        // Écriture volatile sur le registre de configuration
        vol *TIMER_VALUE = period;
        vol *TIMER_CONTROL = 0x01; // Active le timer
    }

    func read_status() -> u32 {
        // Lecture volatile directe du statut matériel
        val status: u32 = vol *TIMER_CONTROL;
        -> status;
    }
}

```

---

## 4. Synthèse des Opérations sur la Mémoire

| Syntaxe | Description | Domaine d'usage |
| --- | --- | --- |
| `*T` | Type pointeur brut vers `T` | Adressage explicite, structures de données |
| `*ptr` | Lecture / Écriture standard | Accès mémoire normal (optimisable par le compilateur) |
| `expr as T` | Cast de type explicite | Conversions numériques, arithmétique de pointeurs (`usize`) |
| `vol *ptr = val` | Écriture volatile directe | Registres MMIO, barrières matérielles, interruptions |
| `vol *ptr` | Lecture volatile directe | Polling de registres matériels, drapeaux partagés |