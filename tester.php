<?php
/**
 * Fonction qui se connecte au socket, envoie une commande et renvoie la réponse.
 *
 * @param string $command La commande à envoyer (terminée par un point-virgule).
 * @return string La réponse reçue depuis le serveur.
 */
function sendCommand($command) {
    $host = '127.0.0.1'; // correspond au hostname défini dans docker-compose
    $port = 8080;        // port sur lequel le serveur C++ écoute

    // Ouverture de la connexion socket
    $socket = fsockopen($host, $port, $errno, $errstr, 10);
    if (!$socket) {
        return "Erreur de connexion au socket: $errno - $errstr\n";
    }
    
    // Envoi de la commande
    fwrite($socket, $command);
    
    // Lecture de la réponse (lecture jusqu'à la fin de la connexion)
    $response = "";
    while (!feof($socket)) {
        $response .= fgets($socket, 1024);
    }
    fclose($socket);
    return $response;
}

// Tableau des commandes à exécuter dans l'ordre
$commands = [
    "CREATE TABLE users;",
    
    "ADD {\"id\": 1, \"name\": \"Alice\", \"age\": 30} IN users;",
    "ADD {\"id\": 2, \"name\": \"Bob\", \"age\": 25} IN users;",
    "ADD {\"id\": 3, \"name\": \"Charlie\", \"age\": 30} IN users;",
    "ADD {\"id\": 4, \"name\": \"Alice\", \"details\": {\"age\": 32, \"city\": \"Paris\"}} IN users;",
    "ADD {\"id\": 5, \"name\": \"Alice\", \"details\": {\"age\": 30, \"city\": \"Paris\"}, \"favorite_flavour\": [\"vanilla\", \"cookies\", \"chocolate\"]} IN users;",
    
    "GET name IN users WHERE id IS 1;",
    "GET name IN users WHERE id NOT 1;",
    "GET name IN users WHERE favorite_flavour HAS vanilla;",
    
    "GET * IN users WHERE age IS 30;",
    "GET * IN users WHERE details.age IS 30;",

    "GET * IN users WHERE details.city IS Paris;",

    "GET name IN users ORDER BY details.age DESC LIMIT 5;",
    "GET name IN users WHERE favorite_flavour HAS vanilla GROUP BY details.city ORDER BY id DESC LIMIT 5;",

    "GET name AS username IN users ORDER BY details.age DESC LIMIT 5;",

    "GET name IN users WHERE age IS 30 AND details.city IS Paris;",

    "UPDATE age = 35 WHERE name IS Alice AND id IS 1 IN users;",

    "GET * IN users WHERE id IS 1;",

    "COUNT users;",
    "COUNT users age:30;",
    "COUNT users details.age:30;",
    
    "REMOVE WHERE age IS 30 AND details.city IS Paris IN users;",

    "REMOVE Alice IN users;",
    
    "REMOVE age:30 IN users;",

    "EMPTY users;",

    "COUNT users;",
    
    "DELETE users;"
];

// Exécution séquentielle des commandes et affichage des réponses
foreach ($commands as $cmd) {
    echo "Envoi de la commande : $cmd\n";
    $response = sendCommand($cmd);
    echo "Réponse du serveur : $response\n\n";
}
?>
