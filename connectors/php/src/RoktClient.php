<?php
declare(strict_types=1);

/**
 * Classe RoktClient
 * Gère une connexion persistante au serveur ROKT via un socket TCP.
 */
class RoktClient {
    private string $host;
    private int $port;
    private $socket; // resource

    public function __construct(string $host = 'rokt_host', int $port = 8080) {
        $this->host = $host;
        $this->port = $port;
    }

    private function connect() {
        $this->socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        if ($this->socket === false) {
            throw new Exception("Erreur de création du socket : " . socket_strerror(socket_last_error()));
        }
        $result = socket_connect($this->socket, $this->host, $this->port);
        if ($result === false) {
            throw new Exception("Erreur de connexion au serveur : " . socket_strerror(socket_last_error($this->socket)));
        }
        return true;
    }

    private function close(): void {
        if ($this->socket !== null) {
            socket_close($this->socket);
            $this->socket = null;
        }
    }

    public function sendCommand(string $command): string {
        if(!$this->connect()) {
            throw new Exception("Erreur lors de la création du socket : " . socket_strerror(socket_last_error($this->socket)));
        }
        // S'assurer que la commande se termine par un point-virgule.
        $command = rtrim($command);
        $written = socket_write($this->socket, $command, strlen($command));
        if ($written === false) {
            $this->close();
            throw new Exception("Erreur d'écriture sur le socket : " . socket_strerror(socket_last_error($this->socket)));
        }
        $response = "";
        while ($out = socket_read($this->socket, 2048)) {
            $response .= $out;
            if (strlen($out) < 2048) break;
        }
        $this->close();
        return $response;
    }
}

/**
 * Classe QueryBuilder
 * Offre une interface fluide pour construire et envoyer des commandes ROKT.
 * Les méthodes update() et remove() ont été étendues pour supporter les conditions.
 */
class QueryBuilder {
    private RoktClient $client;
    private string $command = "";
    // Propriété pour distinguer le type de commande en cours (ex: UPDATE ou REMOVE)
    private string $commandType = "";

    public function __construct(RoktClient $client) {
        $this->client = $client;
    }

    // Méthodes pour CREATE, GET, etc.
    public function create(string $table): self {
        $this->command = "CREATE TABLE " . $table;
        return $this;
    }
    
    public function select(string $fields = "*"): self {
        $this->command = "GET " . $fields;
        return $this;
    }
    
    public function from(string $dataset): self {
        $this->command .= " IN " . $dataset;
        return $this;
    }
    
    public function where(string $field, string $op, string $value): self {
        if (strpos($this->command, "WHERE") === false) {
            $this->command .= " WHERE " . $field . " " . $op . " " . $value;
        } else {
            $this->command .= " AND " . $field . " " . $op . " " . $value;
        }
        return $this;
    }
    
    public function groupBy(string $field): self {
        $this->command .= " GROUP BY " . $field;
        return $this;
    }
    
    public function orderBy(string $field, string $order = "ASC"): self {
        $this->command .= " ORDER BY " . $field . " " . strtoupper($order);
        return $this;
    }
    
    public function limit(int $n): self {
        $this->command .= " LIMIT " . $n;
        return $this;
    }
    
    public function as(string $alias): self {
        $this->command .= " AS " . $alias;
        return $this;
    }
    
    public function add(array|object $data, ?string $uniqueField = null): self {
        $jsonData = json_encode($data);
        $this->command = "ADD " . $jsonData;
        if ($uniqueField !== null) {
            $this->command .= " UNIQUE " . $uniqueField;
        }
        return $this;
    }
    
    public function empty(string $dataset): string {
        $cmd = "EMPTY " . $dataset;
        return $this->client->sendCommand($cmd);
    }
    
    public function delete(string $dataset): string {
        $cmd = "DELETE " . $dataset;
        return $this->client->sendCommand($cmd);
    }
    
    public function count(string $dataset, string $condition = ""): string {
        $cmd = "COUNT " . $dataset;
        if ($condition !== "") {
            $cmd .= " " . $condition;
        }
        return $this->client->sendCommand(rtrim($cmd));
    }
    
    // --- Nouvelle implémentation pour CHANGE ---
    // Cette méthode initialise une commande CHANGE. La table est spécifiée ensuite avec from().
    public function change(string $table): self {
        $this->command = "CHANGE";
        $this->commandType = "CHANGE";
        // On pourra ajouter des affectations via set() et des conditions via where().
        return $this;
    }
    
    // Définit les champs à mettre à jour.
    public function set(string $field, int|float|string|array|object $value): self {
        if (is_array($value) || is_object($value)) {
            $value = json_encode($value);
        }
        if (strpos($this->command, "=") === false) {
            $this->command .= " " . $field . " = " . $value;
        } else {
            $this->command .= ", " . $field . " = " . $value;
        }
        return $this;
    }
    
    // --- Nouvelle implémentation pour REMOVE ---
    // Cette méthode initialise une commande REMOVE.
    public function remove(): self {
        $this->command = "REMOVE";
        $this->commandType = "REMOVE";
        return $this;
    }
    
    // Méthode optionnelle pour ajouter une valeur brute à la commande (ex: "Alice" ou "age:30").
    public function value(string $val): self {
        $this->command .= " " . $val;
        return $this;
    }
    
    // Méthode d'exécution : envoie la commande construite au serveur.
    public function get(): string {
        // On termine toujours la commande par un point-virgule.
        $cmd = rtrim($this->command) . ";";
        // Réinitialisation pour la prochaine commande.
        $this->command = "";
        $this->commandType = "";
        return $this->client->sendCommand($cmd);
    }
}
?>
