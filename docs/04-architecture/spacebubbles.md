# Résolution de Portée, Space Bubbles et Bulles Privées

Dans **Klin**, l'organisation du code repose sur le concept de **Space Bubbles** (Bulles d'Espace). 

Contrairement aux langages qui lient la visibilité des symboles aux fichiers physiques du système de fichiers, Klin gère l'isolation, l'encapsulation et la résolution des symboles exclusivement par la hiérarchie des bulles d'espace.

---

## 1. Déclaration et Accès (`::bubble::subbubble`)

Une bulle regroupe des structures, constantes, alias et fonctions pures. L'accès aux membres ou sous-bulles s'effectue via l'opérateur `::` entre les identifiants, sans préfixer le premier niveau.


### Convention de style et de compilation
Il est de coutume (et recommandé pour des raisons de lisibilité et de parsing par le compilateur) de **déclarer les sous-bulles privées en tout début de bulle**, avant les fonctions et structures publiques.

```klin
crypto::sha256 {

    // Sous-bulles privées placées en début de bulle
    ::_ {
        const INIT_STATE: u32 = 0x6a09e667;
    }

    // Déclarations et structures publiques
    struct Context {
        state: [u32; 8],
        count: u64,
    }

    func hash(data: *u8, len: usize, out_hash: *u8) {
        var ctx: Context;
    }
}

// Depuis un autre espace : accès relatif
func main() {
    var ctx: crypto::sha256::Context;
}
```

---

## 2. La Bulle Privée Anonyme (`_`)

Pour masquer des détails d'implémentation (fonctions auxiliaires, constantes internes, structures de travail), une bulle peut déclarer une sous-bulle privée anonyme au moyen du symbole `_`.

### Règles de Visibilité et d'Accès de `_`

1. **Privatisation liée à la bulle parente** : La bulle `_` est privée pour la **bulle courante** qui la contient (et non pour le fichier source).
2. **Accès direct sans préfixe** : La bulle parente accède **directement** aux éléments de sa sous-bulle privée anonyme, sans aucun préfixe ni qualification (`_::` n'est pas utilisé).
3. **Invisibilité extérieure** : Tout élément défini dans `_` est totalement inaccessible depuis l'extérieur de la bulle parente.

```klin
::sys::allocator {

    // Placé en début de bulle
    ::_ {
        const MAP_ANONYMOUS: u32 = 0x20;

        func align_up(size: usize, align: usize) -> usize {
            -> (size + align - 1) & ~(align - 1);
        }

        func raw_mmap(size: usize) -> *u8 {
            -> null;
        }
    }

    // Élément public de sys::allocator
    func allocate(size: usize) -> *u8 {
        // Appel DIRECT de la fonction privée définie dans la sous-bulle _
        val aligned_size: usize = align_up(size, 8);
        -> raw_mmap(aligned_size);
    }
}
```

---

## 3. Imbrication des Bulles Privées et Transitivité

Lorsqu'une sous-bulle privée contient elle-même des sous-bulles, les règles de visibilité s'appliquent de manière strictement hiérarchique :

1. **Visibilité des sous-bulles publiques :** Toutes les sous-bulles publiques définies à l'intérieur d'une bulle privée sont **visibles par la bulle parente** de cette bulle privée (via le préfixe de la bulle privée nommée).
2. **Herméticité des sous-bulles privées :** En revanche, une sous-bulle privée définie à l'intérieur d'une bulle privée applique strictement sa propre règle d'isolation. La bulle parente de niveau supérieur n'a pas accès aux éléments de cette sous-bulle privée imbriquée.

```klin
::engine::render {

    // Sous-bulle privée
    ::_pipeline {

        // Sous-bulle PRIVÉE de _pipeline : totalement INACCESSIBLE pour engine::render
        ::_ {
            func register_raw_write() { /*...*/ }
        }

        // Sous-bulle PUBLIQUE de _pipeline : ACCESSIBLE par engine::render
        ::shaders {
            func compile_spirv() { /*...*/ }
        }
    }

    func init() {
        // VALIDE : Acces à la sous-bulle publique de la bulle privée
        _pipeline::shaders::compile_spirv();

        // ERREUR DE COMPILATION : _pipeline::_ est strictement privé à _pipeline
        // _pipeline::register_raw_write(); 
    }
}
```

---

## 4. Synthèse de la Visibilité

| Déclaration | Visibilité | Accès depuis la Bulle Parente Directe | Accès depuis la Bulle Grand-Parente | Accès Extérieur |
| --- | --- | --- | --- | --- |
| `::sub { ... }` | Publique | `sub::elem` | `parent::sub::elem` | `elem` qualifié |
| `::_ { ... }` | Privée Anonyme | `elem` *(direct)* | **Impossible** | **Impossible** |
| `::_priv { ... }` | Privée Nommée | `_priv::elem` | **Impossible** | **Impossible** |
| `::_priv { ::pub { ... } }` | Publique d'une Privée | `_priv::pub::elem` | `_priv::pub::elem` | **Impossible** |
| `::_priv { ::_ { ... } }` | Privée d'une Privée | `elem` *(dans `_priv`)* | **Impossible** | **Impossible** |