<?php
require "RoktClient.php";
// On suppose que les classes RoktClient et QueryBuilder sont déjà définies ou incluses.

$client = new RoktClient('127.0.0.1', 8080);
$qb = new QueryBuilder($client);

// --- Création de la table ---
echo "Envoi de la commande : CREATE TABLE users;\n";
$response = $qb->create("users")->get();
echo "Réponse du serveur : $response\n\n";

// --- Commandes ADD ---
// Ici, on reconstruit la commande ADD en formatant les données en JSON
$dataItems = [
    ["id" => 1, "name" => "Alice", "age" => 30],
    ["id" => 2, "name" => "Bob", "age" => 25],
    ["id" => 3, "name" => "Charlie", "age" => 30],
    ["id" => 4, "name" => "Alice", "details" => ["age" => 32, "city" => "Paris"]],
    ["id" => 5, "name" => "Patrick", "details" => ["age" => 30, "city" => "Paris"], "favorite_flavour" => ["vanilla", "cookies", "chocolate"]],
    ["id" => 5, "name" => "Jerome", "details" => ["age" => 30, "city" => "Marseille"], "favorite_flavour" => ["vanilla", "cookies", "chocolate"]]
];

foreach ($dataItems as $data) {
    echo "Envoi de la commande : ADD " . json_encode($data) . " IN users;\n";
    $response = $qb->add($data)->from("users")->get();
    echo "Réponse du serveur : $response\n\n";
}

// --- Commandes GET ---
// Exemple de requête : GET name IN users WHERE id IS 1;
echo "Envoi de la commande : GET name IN users WHERE id IS 1;\n";
$response = $qb->select("name")
               ->from("users")
               ->where("id", "IS", "1")
               ->get();
echo "Réponse du serveur : $response\n\n";

// GET avec négation : on utilise ici l'opérateur '!=' pour 'NOT'
echo "Envoi de la commande : GET name IN users WHERE id != 1;\n";
$response = $qb->select("name")
               ->from("users")
               ->where("id", "!=", "1")
               ->get();
echo "Réponse du serveur : $response\n\n";

// GET sur un champ tableau (par exemple favorite_flavour HAS vanilla)
echo "Envoi de la commande : GET name IN users WHERE favorite_flavour HAS vanilla;\n";
$response = $qb->select("name")
               ->from("users")
               ->where("favorite_flavour", "HAS", "vanilla")
               ->get();
echo "Réponse du serveur : $response\n\n";

// GET avec sélection de toutes les colonnes et condition sur age
echo "Envoi de la commande : GET * IN users WHERE age IS 30;\n";
$response = $qb->select("*")
               ->from("users")
               ->where("age", "IS", "30")
               ->get();
echo "Réponse du serveur : $response\n\n";

// GET sur un sous-champ : details.age
echo "Envoi de la commande : GET * IN users WHERE details.age IS 30;\n";
$response = $qb->select("*")
               ->from("users")
               ->where("details.age", "IS", "30")
               ->get();
echo "Réponse du serveur : $response\n\n";

// GET sur un autre sous-champ : details.city
echo "Envoi de la commande : GET * IN users WHERE details.city IS Paris;\n";
$response = $qb->select("*")
               ->from("users")
               ->where("details.city", "IS", "Paris")
               ->get();
echo "Réponse du serveur : $response\n\n";

// GET avec ORDER BY et LIMIT
echo "Envoi de la commande : GET name IN users ORDER BY details.age DESC LIMIT 5;\n";
$response = $qb->select("name")
               ->from("users")
               ->orderBy("details.age", "DESC")
               ->limit(5)
               ->get();
echo "Réponse du serveur : $response\n\n";

// GET avec GROUP BY, ORDER BY et LIMIT combinés
echo "Envoi de la commande : GET name IN users WHERE favorite_flavour HAS vanilla GROUP BY details.city ORDER BY id DESC LIMIT 5;\n";
$response = $qb->select("name")
               ->from("users")
               ->where("favorite_flavour", "HAS", "vanilla")
               ->groupBy("details.city")
               ->orderBy("id", "DESC")
               ->limit(5)
               ->get();
echo "Réponse du serveur : $response\n\n";

// GET avec alias (AS)
echo "Envoi de la commande : GET name AS username IN users ORDER BY details.age DESC LIMIT 5;\n";
$response = $qb->select("name")
               ->as("username")
               ->from("users")
               ->orderBy("details.age", "DESC")
               ->limit(5)
               ->get();
echo "Réponse du serveur : $response\n\n";

// GET avec plusieurs conditions
echo "Envoi de la commande : GET name IN users WHERE age IS 30 AND details.city IS Paris;\n";
$response = $qb->select("name")
               ->from("users")
               ->where("age", "IS", "30")
               ->where("details.city", "IS", "Paris")
               ->get();
echo "Réponse du serveur : $response\n\n";

// --- Commande CHANGE ---
// La méthode change() du QueryBuilder ne gère pas (encore) l'ajout de conditions,
// nous envoyons donc la commande CHANGE directement via le client.
echo "Envoi de la commande : CHANGE age = 35 WHERE name IS Alice AND id IS 1 IN users;\n";
$response = $qb->change("users")
               ->set("age", 35)
               ->where("name", "IS", "Alice")
               ->where("id", "IS", "1")
               ->from("users")
               ->get();
echo "Réponse du serveur : $response\n\n";

// Vérification du CHANGE
echo "Envoi de la commande : GET * IN users WHERE id IS 1;\n";
$response = $qb->select("*")
               ->from("users")
               ->where("id", "IS", "1")
               ->get();
echo "Réponse du serveur : $response\n\n";

// --- Commandes COUNT ---
echo "Envoi de la commande : COUNT users;\n";
$response = $qb->count("users");
echo "Réponse du serveur : $response\n\n";

echo "Envoi de la commande : COUNT users age:30;\n";
$response = $qb->count("users", "age:30");
echo "Réponse du serveur : $response\n\n";

echo "Envoi de la commande : COUNT users details.age:30;\n";
$response = $qb->count("users", "details.age:30");
echo "Réponse du serveur : $response\n\n";

// --- Commandes REMOVE ---
echo "Envoi de la commande : REMOVE WHERE details.age IS 30 AND details.city IS Paris IN users;\n";
$response = $qb->remove()
               ->where("details.age", "IS", "30")
               ->where("details.city", "IS", "Paris")
               ->from("users")
               ->get();
echo "Réponse du serveur : $response\n\n";

echo "Envoi de la commande : REMOVE Alice IN users;\n";
$response = $qb->remove()
               ->value("Alice")
               ->from("users")
               ->get();
echo "Réponse du serveur : $response\n\n";

echo "Envoi de la commande : REMOVE details.age:30 IN users;\n";
$response = $qb->remove()
               ->value("details.age:30")
               ->from("users")
               ->get();
echo "Réponse du serveur : $response\n\n";

// --- Commande EMPTY ---
echo "Envoi de la commande : EMPTY users;\n";
$response = $qb->empty("users");
echo "Réponse du serveur : $response\n\n";

// Nouvelle vérification avec COUNT après vidage
echo "Envoi de la commande : COUNT users;\n";
$response = $qb->count("users");
echo "Réponse du serveur : $response\n\n";

// --- Commande DELETE ---
echo "Envoi de la commande : DELETE users;\n";
$response = $qb->delete("users");
echo "Réponse du serveur : $response\n\n";
?>
