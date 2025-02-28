Voici un exemple complet de code PHP qui te permet de te connecter à ton serveur ROKT et d’envoyer des requêtes en utilisant un style "fluent" (comme dans ton premier code avec select("*").from()…). Ce client est conçu en classes et couvre toutes les possibilités de ROKT (création, ajout, mise à jour, suppression, vidage, suppression complète, comptage et requêtes GET avec conditions, tri, groupement, etc.).

Ce code se compose de deux classes principales :

1. **RoktClient** : qui gère la connexion au serveur via un socket TCP et envoie les commandes.
2. **QueryBuilder** : qui offre une interface fluide pour construire et envoyer les commandes à ROKT.

Le code est documenté avec des commentaires simples pour être accessible, même à un enfant de 6 ans.

---

```php
<?php
/**
 * RoktClient.php
 *
 * Ce fichier contient deux classes :
 * - RoktClient : gère la connexion au serveur ROKT.
 * - QueryBuilder : offre une interface fluide pour construire des requêtes et couvrir toutes les commandes de ROKT.
 *
 * Pour utiliser ce client, tu peux créer une instance de RoktClient, puis une instance de QueryBuilder
 * pour envoyer des commandes comme GET, ADD, UPDATE, REMOVE, EMPTY, DELETE, COUNT, etc.
 */

/**
 * Classe RoktClient
 *
 * Cette classe se connecte au serveur ROKT via un socket TCP et envoie les commandes.
 */
class RoktClient {
    private $host;
    private $port;

    /**
     * Constructeur.
     *
     * @param string $host Le nom d'hôte du serveur (par exemple "rokt_host").
     * @param int $port Le port sur lequel le serveur écoute (par exemple 8080).
     */
    public function __construct($host = 'rokt_host', $port = 8080) {
        $this->host = $host;
        $this->port = $port;
    }

    /**
     * Envoie une commande au serveur ROKT et renvoie la réponse.
     *
     * @param string $command La commande à envoyer (par exemple "GET * IN users;").
     * @return string La réponse renvoyée par le serveur.
     * @throws Exception Si la connexion ou l'envoi échoue.
     */
    public function sendCommand($command) {
        // Créer un socket TCP/IP
        $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        if ($socket === false) {
            throw new Exception("Erreur de création du socket : " . socket_strerror(socket_last_error()));
        }
        // Se connecter au serveur
        $result = socket_connect($socket, $this->host, $this->port);
        if ($result === false) {
            throw new Exception("Erreur de connexion au serveur : " . socket_strerror(socket_last_error($socket)));
        }
        // Envoyer la commande (en ajoutant un point-virgule si nécessaire)
        $command = rtrim($command) . ";";
        socket_write($socket, $command, strlen($command));
        // Lire la réponse du serveur
        $response = "";
        while ($out = socket_read($socket, 2048)) {
            $response .= $out;
        }
        socket_close($socket);
        return $response;
    }
}

/**
 * Classe QueryBuilder
 *
 * Cette classe permet de construire des requêtes ROKT avec un style fluide.
 * Elle propose des méthodes comme select(), from(), where(), groupBy(), orderBy(), limit(), as(),
 * ainsi que des commandes UPDATE, ADD, REMOVE, EMPTY, DELETE, COUNT.
 */
class QueryBuilder {
    private $client;
    private $command = "";

    /**
     * Constructeur.
     *
     * @param RoktClient $client Une instance de RoktClient.
     */
    public function __construct(RoktClient $client) {
        $this->client = $client;
    }

    // === Méthodes pour GET ===

    /**
     * Commence une requête GET en spécifiant le(s) champ(s) à récupérer.
     *
     * @param string $fields Les champs à récupérer (par défaut "*" pour tous).
     * @return QueryBuilder L'instance pour pouvoir chaîner.
     */
    public function select($fields = "*") {
        $this->command = "GET " . $fields;
        return $this;
    }

    /**
     * Spécifie le dataset (la table) dans lequel chercher.
     *
     * @param string $dataset Le nom du dataset.
     * @return QueryBuilder
     */
    public function from($dataset) {
        $this->command .= " IN " . $dataset;
        return $this;
    }

    /**
     * Ajoute une condition à la requête.
     *
     * Exemple : where("id", "IS", "1") ou where("age", ">", "30")
     *
     * @param string $field Le champ de la condition.
     * @param string $op L'opérateur (IS, NOT, HAS, <, <=, >, >=).
     * @param string $value La valeur à comparer.
     * @return QueryBuilder
     */
    public function where($field, $op, $value) {
        // Si aucune condition n'a encore été ajoutée, on ajoute WHERE,
        // sinon, on ajoute AND (la gestion de OR n'est pas implémentée ici, mais peut être ajoutée)
        if (strpos($this->command, "WHERE") === false) {
            $this->command .= " WHERE " . $field . " " . $op . " " . $value;
        } else {
            $this->command .= " AND " . $field . " " . $op . " " . $value;
        }
        return $this;
    }

    /**
     * Ajoute une clause GROUP BY à la requête.
     *
     * @param string $field Le champ par lequel grouper.
     * @return QueryBuilder
     */
    public function groupBy($field) {
        $this->command .= " GROUP BY " . $field;
        return $this;
    }

    /**
     * Ajoute une clause ORDER BY à la requête.
     *
     * @param string $field Le champ par lequel trier.
     * @param string $order "ASC" (par défaut) ou "DESC".
     * @return QueryBuilder
     */
    public function orderBy($field, $order = "ASC") {
        $this->command .= " ORDER BY " . $field . " " . strtoupper($order);
        return $this;
    }

    /**
     * Ajoute une clause LIMIT à la requête.
     *
     * @param int $n Le nombre maximum d'éléments à retourner.
     * @return QueryBuilder
     */
    public function limit($n) {
        $this->command .= " LIMIT " . $n;
        return $this;
    }

    /**
     * Définit un alias pour le champ sélectionné.
     *
     * Exemple : select("name")->as("username")
     *
     * @param string $alias L'alias à utiliser.
     * @return QueryBuilder
     */
    public function as($alias) {
        $this->command .= " AS " . $alias;
        return $this;
    }

    /**
     * Envoie la requête GET et renvoie la réponse du serveur.
     *
     * @return string La réponse du serveur.
     */
    public function get() {
        $cmd = rtrim($this->command) . ";";
        // Réinitialiser la commande pour la prochaine requête.
        $this->command = "";
        return $this->client->sendCommand($cmd);
    }

    // === Méthodes pour UPDATE ===

    /**
     * Commence une requête UPDATE.
     *
     * Exemple : update("age", "35") indiquera UPDATE age = 35
     *
     * @param string $field Le champ à mettre à jour.
     * @param string $newValue La nouvelle valeur (sans guillemets, c'est à dire telle quelle).
     * @return QueryBuilder
     */
    public function update($field, $newValue) {
        $this->command = "UPDATE " . $field . " = " . $newValue;
        return $this;
    }

    // === Méthodes pour ADD ===

    /**
     * Commence une requête ADD.
     *
     * Le paramètre $jsonData doit être une chaîne nlohmann::json valide.
     * Un champ roktID sera ajouté automatiquement par le serveur.
     *
     * @param string $jsonData La donnée nlohmann::json à ajouter.
     * @return QueryBuilder
     */
    public function add($jsonData) {
        $this->command = "ADD " . $jsonData;
        return $this;
    }

    // === Méthodes pour REMOVE ===

    /**
     * Commence une requête REMOVE.
     *
     * Deux formes sont supportées :
     * - REMOVE <value> IN <dataset> (retire les lignes dont le champ "name" est égal à <value>)
     * - REMOVE WHERE <condition> [AND/OR <condition> ...] IN <dataset>
     *
     * @param string $valueOrCondition La valeur ou la clause WHERE.
     * @return QueryBuilder
     */
    public function remove($valueOrCondition) {
        $this->command = "REMOVE " . $valueOrCondition;
        return $this;
    }

    // === Méthodes pour EMPTY (vider une table) ===

    /**
     * Envoie une commande EMPTY pour vider le dataset (table) sans supprimer le dossier.
     *
     * @param string $dataset Le nom du dataset.
     * @return string La réponse du serveur.
     */
    public function empty($dataset) {
        $cmd = "EMPTY " . $dataset . ";";
        return $this->client->sendCommand($cmd);
    }

    // === Méthodes pour DELETE (supprimer complètement une table) ===

    /**
     * Envoie une commande DELETE pour supprimer complètement le dataset (table).
     *
     * @param string $dataset Le nom du dataset.
     * @return string La réponse du serveur.
     */
    public function delete($dataset) {
        $cmd = "DELETE " . $dataset . ";";
        return $this->client->sendCommand($cmd);
    }

    // === Méthodes pour COUNT ===

    /**
     * Envoie une commande COUNT pour compter le nombre de lignes dans un dataset,
     * éventuellement filtré par une condition (par exemple "age:30").
     *
     * @param string $dataset Le nom du dataset.
     * @param string $condition (optionnel) Une condition sous la forme "key:value".
     * @return string La réponse du serveur (au format nlohmann::json).
     */
    public function count($dataset, $condition = "") {
        $cmd = "COUNT " . $dataset;
        if ($condition != "") {
            $cmd .= " " . $condition;
        }
        return $this->client->sendCommand(rtrim($cmd) . ";");
    }
}
?>
```

---

## Exemple d'utilisation

Voici un exemple de script PHP qui utilise ces classes pour interagir avec le serveur ROKT. Ce script envoie différentes commandes pour couvrir toutes les possibilités.

```php
<?php
require_once 'RoktClient.php';  // Contient les classes RoktClient et QueryBuilder

try {
    // Créer une instance de RoktClient (adapter l'hôte et le port si nécessaire)
    $client = new RoktClient("rokt_host", 8080);
    
    // Créer une instance de QueryBuilder pour construire et envoyer des requêtes
    $query = new QueryBuilder($client);
    
    // Exemple 1 : Créer une table "users"
    $response = $client->sendCommand("CREATE TABLE users;");
    echo "CREATE TABLE Response: " . $response . "\n";
    
    // Exemple 2 : Ajouter des informations
    $data1 = '{"id": 1, "name": "Alice", "age": 30}';
    $response = $query->add($data1)->from("users")->get();
    echo "ADD Response 1: " . $response . "\n";
    
    $data2 = '{"id": 2, "name": "Bob", "age": 25}';
    $response = $query->add($data2)->from("users")->get();
    echo "ADD Response 2: " . $response . "\n";
    
    $data3 = '{"id": 3, "name": "Charlie", "age": 30}';
    $response = $query->add($data3)->from("users")->get();
    echo "ADD Response 3: " . $response . "\n";
    
    $data4 = '{"id": 1, "name": "Alice", "details": {"age": 32, "city": "Paris"}}';
    $response = $query->add($data4)->from("users")->get();
    echo "ADD Response 4: " . $response . "\n";
    
    $data5 = '{"id": 1, "name": "Alice", "details": {"age": 30, "city": "Paris"}, "favorite_flavour": ["vanilla", "cookies", "chocolate"]}';
    $response = $query->add($data5)->from("users")->get();
    echo "ADD Response 5: " . $response . "\n";
    
    // Exemple 3 : Récupérer des informations avec GET
    $response = $query->select("name")->as("username")->from("users")->orderBy("details.age", "DESC")->limit(5)->get();
    echo "GET Response: " . $response . "\n";
    
    // Exemple 4 : Mettre à jour des informations
    $response = $query->update("age", "35")->where("name", "IS", "Alice")->where("id", "IS", "1")->from("users")->get();
    echo "UPDATE Response: " . $response . "\n";
    
    // Exemple 5 : Supprimer des informations
    $response = $query->remove("Alice")->from("users")->get();
    echo "REMOVE Response: " . $response . "\n";
    
    // Exemple 6 : Vider la table
    $response = $query->empty("users");
    echo "EMPTY Response: " . $response . "\n";
    
    // Exemple 7 : Supprimer la table
    $response = $query->delete("users");
    echo "DELETE Response: " . $response . "\n";
    
    // Exemple 8 : Compter les lignes
    $response = $query->count("users", "age:30");
    echo "COUNT Response: " . $response . "\n";
    
} catch (Exception $e) {
    echo "Erreur: " . $e->getMessage() . "\n";
}
?>
```

---

## Explications pour un enfant de 6 ans

Imagine que ton ordinateur est un grand téléphone magique qui peut parler à un serveur secret appelé ROKT.  
- **RoktClient** est comme ton téléphone magique : il sait comment composer le numéro (l'adresse et le port) et envoyer des messages.
- **QueryBuilder** est comme un petit assistant qui t’aide à écrire des messages en langage facile, par exemple « montre-moi le nom de tous les jouets » ou « ajoute ce jouet dans le tiroir des jouets ».
- Tu peux demander au serveur de créer des tiroirs (tables), d’y mettre des jouets (informations), de chercher des jouets par caractéristiques, de modifier les informations, ou même de compter combien de jouets il y a.
- Toutes ces commandes sont envoyées sous forme de texte secret au serveur, et le serveur te renvoie la réponse dans une jolie boîte (un objet nlohmann::json) qui te dit ce qui a été fait.

Ce code PHP te permet de parler à ton serveur ROKT de manière simple et amusante, en utilisant des méthodes comme **select()**, **from()**, **where()**, **add()**, **update()**, **remove()**, **empty()**, **delete()**, et **count()**.