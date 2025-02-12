# ROKT Sources

## Introduction

ROKTSql est un systeme de base de donnees rapide et dynamique permettant le traitement, analyse et gestion de donnees.
Les donnees dans ROKTSQL peuvent etre fixe comme une table SQL ou dynamique comme en NoSQL.

De plus, les donnees peuvent etre mises dans des tables speciales, ditent *rotatives*, qui permettent de decharger une table soit en fonction du nombre de lignes dans la table, soit en fonction de la memoire que celle-ci peut prendre.
Il existe aussi les tables de forme *simple*.

## Inventaire

### Gestion des configuration

#### Generalite
- Format YML
- Config fixe > config.yml
```yml
users:
    - DATABASE_ROOT_USER: root
    - DATABASE_ROOT_PASSWORD: 
security:
    - level: 0
    - CIPHERING: AES-128-CTR
    - ENCRYPTION_IV: 3567545563424789
    - ENCRYPTION_KEY: 45MHaxg35N6Xev2rWFhSU45DMcQPrbt

```

- Config dynamique > datasets.yml
```yml
datasets:
    - infos: 
        type: SIMPLE
    - users: 
        type: SIMPLE
    - messages: 
        type: ROTATE
        size: 10KB
        nb_rotation: 2
```

#### Fonctions
- Afficher la config
- Lire la config
- Modifier une config
- Supprimer une config
- Ajouter une config

### Datasets
#### Generalite
Un dataset est une table dans laquelle les donnees seront stockees.
Les tables sont chiffrees et binarisees.

#### Fonctions
- Lire la table
- Chercher une ligne
- Ajouter une ligne
- Modifier une ligne
- Supprimer une ligne

## Détails des dossiers
1. `src/` (Source)
- Contient le code source du projet.
- Chaque module ou fonctionnalité peut avoir son propre fichier source.
- Si le projet est grand, regroupez les modules dans des sous-dossiers.
2. `include/` (Fichiers d'en-tête)
- Contient tous les fichiers `.h` utilisés par le projet.
- Chaque fichier `.h` correspond à un fichier `.c` dans le dossier src/.
- Les fichiers `.h` servent à déclarer les fonctions, structures, macros et constantes.
3. `lib/` (Bibliothèques)
- Contient les bibliothèques externes nécessaires au projet, sous forme de fichiers `.a`, `.so` ou même leurs sources.
4. `build/` (Compilés)
- Dossier dédié aux fichiers compilés et exécutables.
- Géré par votre système de build (e.g., `Makefile`, `CMake`).
Permet de séparer le code source des fichiers générés.
5. `tests/` (Tests)
- Regroupe tous les fichiers de test, notamment les tests unitaires.
- Utilisez un framework de tests pour C, comme Unity, Check ou CMocka.
6. `docs/` (Documentation)
- Contient la documentation du projet.
- Utilisez des outils comme Doxygen pour générer automatiquement une documentation à partir de commentaires dans le code.
7. Fichiers à la racine
- `Makefile` : Automatisation du processus de compilation.
- `README.md` : Explications sur le projet, comment l’utiliser, et les instructions de compilation.

## Vocabulaire

Terme | Equivalent SQL | Explication
--- | --- | ---
Dataset | TABLE | Definition des donnees contenues dans une table
Data | Row | La donnee contenue dans un dataset

## Usage

### Namespaces

#### Response service
#### Object
#### Dataset
#### Data
#### Encrypt

## Tests

test_socket_connection -> Verifie la connection au serveur