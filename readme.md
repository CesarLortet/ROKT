# Documentation Utilisateur du Projet ROKT

Bienvenue dans le guide du projet ROKT !  
Ce guide va t'expliquer, étape par étape et avec des mots simples, comment fonctionne notre projet. Imagine que c'est comme un grand jeu où tu peux créer des "tables" (comme des tiroirs où tu ranges des informations), ajouter des informations dedans, les chercher, les modifier, ou même les enlever. Tout est sécurisé par un "magicien" qui chiffre (cache) les informations pour que personne ne puisse les lire sans la bonne clé secrète.

---

## Table des Matières

1. [Introduction Générale](#introduction-générale)
2. [Les Principes de Base](#les-principes-de-base)
   - [Que fait ROKT ?](#que-fait-rokt-)
   - [Les Datasets et Tables](#les-datasets-et-tables)
3. [L'Environnement et l'Installation](#lenvironnement-et-linstallation)
4. [Les Commandes du Jeu](#les-commandes-du-jeu)
   - [Créer une Table](#créer-une-table)
   - [Ajouter des Informations (ADD)](#ajouter-des-informations-add)
   - [Chercher des Informations (GET)](#chercher-des-informations-get)
   - [Mettre à Jour des Informations (CHANGE)](#mettre-à-jour-des-informations-change)
   - [Supprimer des Informations (REMOVE)](#supprimer-des-informations-remove)
   - [Vider une Table (EMPTY)](#vider-une-table-empty)
   - [Supprimer une Table (DELETE)](#supprimer-une-table-delete)
   - [Compter des Lignes (COUNT)](#compter-des-lignes-count)
5. [Le Système de Sécurité et le Chiffrement](#le-système-de-sécurité-et-le-chiffrement)
6. [Architecture du Projet](#architecture-du-projet)
7. [Exemples et Scénarios d'Utilisation](#exemples-et-scénarios-dutilisation)
8. [FAQ - Questions Fréquentes](#faq---questions-fréquentes)
9. [Conclusion](#conclusion)

---

## Introduction Générale

ROKT est un projet qui te permet de **créer**, **ajouter**, **chercher**, **mettre à jour** et **supprimer** des informations dans des "tables" (aussi appelées **datasets**).  
Imagine que tu as une grande boîte de jouets. Tu peux y ranger tes jouets dans des tiroirs (tables). Chaque tiroir peut contenir plusieurs jouets (lignes) avec des informations comme le nom, l'âge, ou même une petite histoire sur le jouet !

Mais pour garder ces jouets bien cachés, notre projet utilise un "magicien" spécial (le chiffrement) qui rend les informations secrètes. Seule la personne qui connaît la clé secrète peut voir ce qu'il y a dedans.

---

## Les Principes de Base

### Que fait ROKT ?

- **Créer une Table** : C'est comme fabriquer un nouveau tiroir pour ranger tes jouets.
- **Ajouter des Informations** : Tu peux y mettre des jouets avec des informations, comme leur couleur, leur taille, etc.
- **Chercher des Informations** : Tu peux demander au système de te montrer seulement les jouets qui ont une certaine caractéristique.
- **Mettre à Jour des Informations** : Si tu veux changer la couleur d'un jouet, tu peux le faire.
- **Supprimer des Informations** : Tu peux retirer un jouet du tiroir si tu ne l'aimes plus.
- **Vider une Table** : Tu peux vider complètement un tiroir sans le jeter.
- **Supprimer une Table** : Tu peux jeter tout le tiroir si tu n'en as plus besoin.
- **Compter des Lignes** : Tu peux demander combien de jouets se trouvent dans un tiroir ou combien répondent à certains critères.

### Les Datasets et Tables

- **Dataset** : C'est une table ou un tiroir où tu ranges des informations.
- **Ligne** : C'est une seule information (comme un jouet) qui contient plusieurs propriétés (comme son nom, son âge, etc.).
- **Champ** : C'est une propriété dans une ligne (par exemple, le "nom" du jouet).

---

## L'Environnement et l'Installation

Pour utiliser ROKT, tu dois installer et lancer le projet dans un environnement informatique (souvent dans un conteneur Docker).  
Les étapes d'installation sont :

1. **Installer Docker** : C'est un outil qui permet de lancer des applications comme des petits conteneurs.
2. **Télécharger le Projet ROKT** : Tu obtiens tout le code source du projet.
3. **Construire l'Image Docker** : Le projet est compilé (transformé en un programme exécutable) et configuré.
4. **Lancer le Conteneur** : Une fois lancé, le conteneur écoute sur un port (par exemple, le port 8080) pour recevoir des commandes.

---

## Les Commandes du Jeu

Chaque commande est une phrase magique que tu envoies au système pour lui dire ce qu'il doit faire.

### Créer une Table

**Commande :**  
```
CREATE TABLE users;
```

**Explication :**  
Ceci crée un nouveau tiroir appelé "users" pour ranger des informations.

### Ajouter des Informations (ADD)

**Commande :**  
```
ADD {"id": 1, "name": "Alice", "age": 30} IN users;
```

**Explication :**  
- La partie entre accolades `{...}` est une information (comme un jouet).
- Un identifiant spécial appelé `roktID` est automatiquement ajouté pour identifier chaque ligne.
- Cette commande met ce jouet dans le tiroir "users".

### Chercher des Informations (GET)

**Commande :**  
```
GET name AS username IN users ORDER BY details.age DESC LIMIT 5;
```

**Explication :**  
- **GET name** : Tu demandes à voir le champ "name" (le nom du jouet).
- **AS username** : Tu veux que ce nom soit affiché sous le nom "username".
- **IN users** : Tu regardes dans le tiroir "users".
- **ORDER BY details.age DESC** : Tu veux que les jouets soient triés par "details.age" (par exemple, l'âge dans une partie spéciale d'information) en ordre décroissant (du plus grand au plus petit).
- **LIMIT 5** : Tu ne veux voir que 5 jouets au maximum.
- Seules les lignes qui ont la donnée "details.age" seront utilisées pour le tri.

### Mettre à Jour des Informations (CHANGE)

**Commande :**  
```
CHANGE age = 35 WHERE name IS Alice AND id IS 1 IN users;
```

**Explication :**  
Cette commande trouve le jouet dans le tiroir "users" qui s'appelle "Alice" et qui a l'identifiant 1, puis change son âge pour 35.

### Supprimer des Informations (REMOVE)

**Commande (simple) :**  
```
REMOVE Alice IN users;
```

**Explication :**  
Si tu écris REMOVE suivi d'une valeur sans WHERE, le système considère que tu veux retirer toutes les lignes où le champ par défaut (ici "name") est égal à "Alice".

**Commande (avec conditions multiples) :**  
```
REMOVE WHERE age IS 30 AND details.city IS Paris IN users;
```

**Explication :**  
Cette commande supprime tous les jouets du tiroir "users" qui ont 30 ans et qui viennent de "Paris".

### Vider une Table (EMPTY)

**Commande :**  
```
EMPTY users;
```

**Explication :**  
Cela vide complètement le tiroir "users" (les jouets sont retirés, mais le tiroir reste).

### Supprimer une Table (DELETE)

**Commande :**  
```
DELETE users;
```

**Explication :**  
Cette commande supprime complètement le tiroir "users" et toutes les informations qu'il contient.

### Compter des Lignes (COUNT)

**Commande :**  
```
COUNT users details.age:30;
```

**Explication :**  
Cette commande te donne le nombre de jouets dans le tiroir "users" qui ont "details.age" égal à 30.  
Le résultat est renvoyé sous forme d'un objet JSON comme :  
```json
{"count": 2}
```

---

## Le Système de Sécurité et le Chiffrement

Imagine que tes jouets sont gardés dans un coffre-fort magique !  
- **Chiffrement du contenu :**  
  Toutes les informations sont cachées (chiffrées) avant d'être stockées dans un fichier.  
- **Chiffrement des noms :**  
  Même le nom du tiroir (et le nom du fichier de configuration) est transformé en un code secret, pour que personne ne puisse deviner ce qu'il y a dedans sans connaître la clé magique.
- **La Clé Magique :**  
  Le projet utilise une clé secrète (passphrase) pour transformer les données en un message secret et les retransformer en données lisibles seulement par ceux qui connaissent la clé.

---

## Architecture du Projet

Le projet est construit comme une grande machine à commandes qui comprend plusieurs parties :

- **Serveur Socket en C++ :**  
  Le cœur du projet est un programme écrit en C++ qui écoute sur un port (par exemple, 8080).  
  Il reçoit les commandes (les phrases magiques) et les traite.

- **Chaîne de Responsabilité :**  
  Chaque commande est envoyée à une chaîne de "handlers" (gestionnaires) qui décident qui va s'en charger.  
  Par exemple, il y a un handler pour ADD, un pour GET, un pour CHANGE, etc.  
  C'est comme si chaque commande passait d'un garde à l'autre, jusqu'à ce que l'un d'eux sache quoi faire.

- **Chiffrement (EncryptService) :**  
  Ce composant transforme les messages en secrets et les retransforme en texte lisible.  
  Il est utilisé pour les contenus stockés et même pour les noms des fichiers et dossiers.

- **Gestion des Datasets :**  
  Les datasets (ou tables) sont stockés dans des fichiers.  
  Des commandes permettent de créer, modifier et supprimer ces fichiers.

---

## Exemples et Scénarios d'Utilisation

1. **Créer un Dataset et y Ajouter des Informations :**
   - Tu écris :  
     ```
     CREATE TABLE users;
     ```
     Cela crée un nouveau tiroir pour ranger tes jouets.
   - Ensuite, tu ajoutes des informations :  
     ```
     ADD {"id": 1, "name": "Alice", "age": 30} IN users;
     ```
     Le système ajoute automatiquement un identifiant secret `roktID` à la ligne.

2. **Chercher des Informations avec Tri et Limite :**
   - Tu peux demander :  
     ```
     GET name AS username IN users ORDER BY details.age DESC LIMIT 5;
     ```
     Cela te montre le nom de chaque jouet (avec l'alias "username") dans le tiroir "users", triés par l'âge dans la partie "details". Seuls les jouets qui ont cette information (par exemple, ceux qui ont "details.age") seront affichés.

3. **Mettre à Jour des Informations :**
   - Pour changer l'âge d'un jouet, tu peux écrire :  
     ```
     CHANGE age = 35 WHERE name IS Alice AND id IS 1 IN users;
     ```
     Le système va chercher le jouet qui s'appelle "Alice" et qui a l'identifiant 1 et changer son âge à 35.

4. **Supprimer des Informations avec Conditions Multiples :**
   - Pour enlever un jouet ou plusieurs, tu peux écrire :  
     ```
     REMOVE WHERE age IS 30 AND details.city IS Paris IN users;
     ```
     Cela enlève les jouets qui ont 30 ans et qui viennent de Paris.

5. **Vider ou Supprimer une Table :**
   - Vider la table (garder le tiroir vide) :  
     ```
     EMPTY users;
     ```
   - Supprimer la table (jeter le tiroir) :  
     ```
     DELETE users;
     ```

6. **Compter des Lignes :**
   - Pour savoir combien de jouets ont une certaine caractéristique, tu peux écrire :  
     ```
     COUNT users details.age:30;
     ```
     et le système te répondra par exemple :  
     ```json
     {"count": 2}
     ```

---

## FAQ - Questions Fréquentes

**Q : Pourquoi mon système utilise-t-il des mots comme "chiffrer" et "décrypter" ?**  
R : C'est pour protéger les informations, un peu comme si tu mettais tes jouets dans un coffre fort et que seul toi connaissais le code secret pour l'ouvrir !

**Q : Que signifie "roktID" ?**  
R : C'est un identifiant unique généré automatiquement pour chaque ligne. C'est comme donner un numéro spécial à chaque jouet pour qu'on puisse toujours le reconnaître.

**Q : Et si je fais une erreur dans ma commande ?**  
R : Le système te renverra un message d'erreur pour t'aider à corriger ta commande.

---

## Conclusion

Le projet ROKT est un outil puissant pour gérer des informations de manière sécurisée.  
- Il te permet de **créer**, **ajouter**, **chercher**, **mettre à jour** et **supprimer** des informations dans des tables (datasets).  
- Toutes les données sont protégées par un chiffrement, ce qui signifie qu'elles sont cachées et ne peuvent être lues que par ceux qui connaissent la clé secrète.  
- Le système utilise une chaîne de responsabilités pour traiter chaque commande, ce qui rend le code modulaire et facile à maintenir.

Nous espérons que ce guide t'aide à comprendre et utiliser le projet ROKT, même si tu es très jeune. Amuse-toi bien en explorant et en apprenant comment fonctionne ce grand jeu informatique !

---