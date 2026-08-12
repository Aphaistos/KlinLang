# Assembleur en Ligne (`asm { ... }`)

Dans **Klin**, l'accès aux instructions machines brutes, à la gestion des interruptions et à la manipulation directe des registres processeur s'effectue au moyen du bloc d'assembleur en ligne `asm`.

L'intégration de l'assembleur se fait sans passer par des fichiers d'assemblage externes pour les séquences courtes, garantissant un contrôle précis au niveau instruction.

---

## 1. Syntaxe Générale

La syntaxe d'un bloc `asm` prend la forme suivante :

```klin
asm { "instructions" : entrées : sorties }

```

* **Instructions** : Une ou plusieurs instructions assembleur représentées sous forme de chaîne de caractères. Les instructions multiples sont séparées par des saut de ligne `\n` ou des points-virgules `;`.
* **Entrées** : Liste des variables ou expressions Klin transmises aux registres assembleur.
* **Sorties** : Liste des variables Klin recevant le résultat des opérations effectuées dans le bloc.

---

## 2. Utilisation Monoligne et Forme Courte (`=>`)

Pour des instructions processeur simples sans passage de paramètres (ex: `hlt`, `cli`, `sti`, `nop`), la section des entrées/sorties peut être entièrement omise.

Combiné à la syntaxe de fonction courte `=>`, cela permet de définir des abstractions matérielles minimales en une seule ligne :

```klin
// Abstractions d'instructions processeur x86
func hlt() => asm { "hlt" };
func cli() => asm { "cli" };
func sti() => asm { "sti" };
func nop() => asm { "nop" };

```

---

## 3. Registres, Entrées et Sorties

Lorsque l'assembleur interagit avec des variables Klin, les contraintes de registres ou d'opérandes sont spécifiées dans les sections d'entrées et de sorties.

```klin
::arch::x86 {

    // Lecture du registre de contrôle CR3 (adresse de la page directory)
    func read_cr3() -> usize {
        var cr3_val: usize;
        asm { "mov %cr3, %rax" : : cr3_val };
        -> cr3_val;
    }

    // Écriture d'une valeur dans un port d'E/S (I/O Port)
    func outb(port: u16, val: u8) {
        asm { "outb %al, %dx" : port, val : };
    }
}

```

---

## 4. Précautions d'Optimisation et Barrières

Le compilateur Klin traite les blocs `asm` comme de l'assembleur volatil par défaut :

1. **Pas de réordonnancement** : Le compilateur ne déplacera pas une instruction `asm` par-dessus un accès mémoire volatile (`vol`).
2. **Effets de bord** : Les instructions contenues dans un bloc `asm` sont conservées même si les variables de sortie ne semblent pas lues immédiatement dans le code Klin environnant.

---

## 5. Synthèse des Formes `asm`

| Forme | Syntaxe | Usage principal |
| --- | --- | --- |
| **Instruction Seule** | `asm { "inst" }` | Control processeur sans E/S (`hlt`, `cli`, `vmmcall`) |
| **Avec Entrées** | `asm { "inst" : inputs : }` | Écriture vers registres/ports (`outb`, `wrmsr`) |
| **Avec Sorties** | `asm { "inst" : : outputs }` | Lecture de registres spécifiques (`read_cr3`, `rdtsc`) |
| **Complet (E/S)** | `asm { "inst" : inputs : outputs }` | Calculs assembleur dédiés, opérations atomiques |