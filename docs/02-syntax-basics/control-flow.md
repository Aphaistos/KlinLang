# Structures de Contrôle de Flux

Dans **Klin**, le contrôle de flux repose sur des structures volontairement simples, lisibles et sans ambiguïté syntaxique. Les structures conditionnelles et de boucle évitent la verbosité tout en conservant un typage strict et explicite.

---

## 1. Conditions (`if` / `else`)

L'instruction `if` évalue une condition booléenne. Les parenthèses autour de la condition sont optionnelles, mais les accolades `{}` délimitant les blocs sont obligatoires.

```klin
func check_status(code: u32) {
    if code == 0 {
        // Traitement de succès
    } else if code == 1 {
        // Traitement d'avertissement
    } else {
        // Gestion d'erreur
    }
}

```

---

## 2. Boucles par Intervalle (`for`)

Inspirées de la notation fonctionnelle (style OCaml), les boucles `for` dans Klin utilisent une flèche d'intervalle `->` pour spécifier la borne supérieure (exclusive).

L'index de boucle est automatiquement inféré par le compilateur en fonction des bornes ou du contexte d'utilisation, rendant l'annotation de type optionnelle.

```klin
::drivers::vga {

    func clear_screen() {
        // 'i' est automatiquement inféré comme usize
        for i = 0 -> 2000 {
            *(0xB8000 as *u16 + i) = 0;
        }
    }

    func fill_buffer(buffer: []u8, val: u8) {
        for i: usize = 0 -> buffer.len {
            buffer[i] = val;
        }
    }
}

```

---

## 3. Boucles Conditionnelles (`while`) et Inconditionnelles (`loop`)

### A. Boucle Conditionnelle (`while`)

La boucle `while` réévalue une expression booléenne à chaque itération. Elle est fréquemment utilisée pour le *polling* de registres matériels ou la synchronisation I/O.

```klin
func wait_for_vsync() {
    // Attente active sur un registre vidéo MMIO
    while (vol *0x3DA as *u8 & 0x08) == 0 {
        asm { "nop" }
    }
}

```

### B. Boucle Infinie (`loop`)

Pour les tâches réseau, la boucle principale d'un noyau (*kernel loop*) ou le traitement ininterrompu d'interruptions, Klin fournit le mot-clé dédié `loop`.

```klin
func kernel_main() {
    init_hardware();

    // Boucle infinie d'exécution
    loop {
        handle_interrupts();
        asm { "hlt" }
    }
}

```

---

## 4. Filtrage par Motif (`match`)

L'instruction `match` permet de faire de la disjonction de cas exhaustive sur des valeurs numériques, des énumérations ou des masques de constantes.

Chaque branche s'écrit sous la forme `valeur => { ... }` ou `valeur => expression`. La branche par défaut est représentée par le tiret bas `_`.

```klin
enum PowerState : u8 {
    Off     = 0,
    Sleep   = 1,
    Running = 2,
}

func handle_power_event(state: PowerState) {
    match state {
        PowerState::Off => {
            shutdown_peripherals();
        }
        PowerState::Sleep => prepare_sleep_mode();
        PowerState::Running => ->;
        _ => panic();
    }
}

```

---

## 5. Synthèse des Structures de Contrôle

| Instruction | Syntaxe | Usage principal |
| --- | --- | --- |
| **`if` / `else**` | `if cond { ... } else { ... }` | Branchement conditionnel standard |
| **`for`** | `for i = min -> max { ... }` | Parcours d'intervalles et de séquences mémoire |
| **`while`** | `while cond { ... }` | Boucle conditionnelle, *polling* matériel |
| **`loop`** | `loop { ... }` | Boucle infinie bas niveau (*kernel loop*, tâches fond) |
| **`match`** | `match val { pattern => expr, _ => ... }` | Filtrage par motif et décodage d'énumérations/états |