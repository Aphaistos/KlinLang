# Modules, Fichiers et Importations (`mod` et `imp`)

Dans **Klin**, le système d'organisation du code est strictement séparé de l'arborescence des fichiers sur le disque. Le compilateur orchestre l'assemblage du code source à l'aide de deux concepts fondamentaux :
1. **Les Modules (`mod`)** : Unités de compilation logiques qui déclarent la structure globale du projet et lient les fichiers sources.
2. **Les Importations (`imp`)** : Directives de chargement de symboles inter-bulles garantissant une compilation en **une seule passe** sans dépendances cycliques ni directives de préprocesseur.

---

## 1. Déclaration de Module (`mod`) et Liaison de Fichiers (`->`)

Chaque programme ou bibliothèque en Klin comporte au moins un module (le module principal). Un module est déclaré à l'aide du mot-clé `mod` suivi du nom qualifié de sa bulle racine (ex: `mod ::std`).

À l'intérieur du bloc `mod`, l'instruction de lien `->` indique au compilateur quels fichiers sources (`.kln`) constituent la totalité du module. Les fichiers ne sont que de simples **contenants de code** : ils n'introduisent aucun espace de nommage implicite.

```klin
// Déclaration du module standard et assemblage de ses fichiers
mod ::std {
    -> "std/sys.kln"
    -> "std/io.kln"
    -> "std/net.kln"
}

```

### Processus de Compilation

Lors de l'appel du compilateur Klin, seuls les fichiers d'entrée contenant la définition des modules (`mod`) doivent lui être transmis. Le compilateur lit les déclarations `mod`, résout les liens `-> "path"` et charge le code source en mémoire.

> **Note :** Le module `::std` est automatiquement inclus par le compilateur lors de la phase d'assemblage du graphe de dépendances.

---

## 2. Importation de Symboles (`imp`)

Une fois les modules chargés, l'instruction `imp` rend accessibles les symboles, structures, fonctions et constantes définis dans les **Space Bubbles** d'autres modules ou du même module.

L'importation s'effectue au niveau des bulles d'espace, en spécifiant le chemin d'accès qualifié.

```klin
imp sys::ports;
imp arch::x86::idt;

::vga {
    const BUFFER_ADDR: usize = 0xB8000;

    func write_char(c: u8, color: u8) {
        // Utilisation des fonctions importées depuis sys::ports
        ports::outb(0x3D4, 0x0E);
    }
}

```

---

## 3. Formes d'Importation de Modules (`imp module`)

Klin propose deux modes d'importation selon que l'on souhaite accéder uniquement aux déclarations directes d'un module ou à l'ensemble de son arborescence de sous-bulles :

### A. Importation de Racine (`imp module;`)

Importer un module sans spécifier de sous-bulle charge uniquement les fonctions, constantes et types déclarés au premier niveau du module, sans importer le contenu des fichiers enfants ou sous-bulles qu'il contient.

```klin
// Importe uniquement la bulle racine du module std
imp std;

func main() {
    // Accessible si défini à la racine de std
    val version: u32 = std::VERSION;
}

```

### B. Importation Complète (`imp module::*;`)

L'utilisation de la wildcard `::*;` importe le module dans son intégralité : la bulle racine ainsi que toutes les sous-bulles publiques exposées à travers tous les fichiers liés par le module.

```klin
// Importe le module std et l'ensemble de ses sous-bulles enfants (sys, net, io...)
imp std::*;

func main() {
    std::sys::ports::outb(0x80, 0x00);
    var socket: std::net::TcpSocket;
}

```

---

## 4. Synthèse du Système de Modules

| Syntaxe | Rôle | Portée / Effet |
| --- | --- | --- |
| `mod ::name { -> "file.kln" }` | Déclaration de module | Associe des fichiers physiques `.kln` à une bulle racine |
| `imp bubble::sub;` | Importation ciblée | Importe une bulle d'espace spécifique |
| `imp module;` | Importation racine | Importe uniquement les symboles du niveau racine du module |
| `imp module::*;` | Importation globale | Importe le module et l'intégralité de ses sous-bulles |