# ROKT Project

## Overview

ROKT is a high-performance server designed to process structured commands over a TCP socket connection. It supports a variety of operations (e.g., `CREATE`, `ADD`, `GET`, etc.) on datasets, leveraging a multi-threaded architecture with `epoll` for efficient network handling and a priority-based task queue for command processing. The project is implemented in C++17 and aims to provide a robust, scalable solution for data manipulation and retrieval.

### Key Features
- **Command Processing**: Supports complex commands with a flexible handler system.
- **Scalability**: Uses `epoll` and a thread pool to handle multiple concurrent connections.
- **Priority Queue**: Tasks are prioritized based on command type (e.g., `CREATE` > `GET`).
- **Configuration**: Configurable via JSON file and environment variables.
- **Error Handling**: Detailed response objects with status codes and messages.

---

## Project Structure

### Core Files
- **`main.cpp`**: Entry point; initializes the server, handlers, and networking.
- **`SyncService.h` / `SyncService.cpp`**: Manages network connections, task queue, and thread pool.
- **`RoktResponseObject.h`**: Defines the response structure for command execution.
- **`Config.h` / `Config.cpp`**: Handles configuration loading from JSON and environment variables.

### Test Files
- **`rokt_load_test.cpp`**: Load test script for ROKT, inserting 1 million rows.
- **`sql_load_test.cpp`**: Equivalent load test using SQLite for benchmarking.

---

## Core Components

### `main.cpp`

#### Description
The main entry point of the ROKT server. It sets up signal handling, initializes the configuration, creates the handler map, configures the TCP socket, and starts the `SyncService`.

#### Key Functions
- **`signal_handler(int sig)`**: Handles `SIGINT` and `SIGTERM` for graceful shutdown.
- **`createHandlerMap(RoktService* roktService)`**: Creates a `HandlerMap` associating commands to their respective handlers.
- **`main()`**: Orchestrates server startup and shutdown.

#### Usage
```bash
g++ -o rokt main.cpp SyncService.cpp RoktService.cpp Config.cpp LogService.cpp -pthread
./rokt
```

---

### `SyncService.h` / `SyncService.cpp`

#### Description
`SyncService` is the heart of the server, responsible for:
- Monitoring network events using `epoll`.
- Managing a pool of worker threads.
- Processing tasks via a priority queue with command-specific priorities.

#### Key Components
- **`Task`**: Struct representing a task with `socket`, `request`, and `priority`.
- **`HandlerMap`**: Maps command strings to `CommandHandler` instances for O(1) dispatch.
- **`workerLoop()`**: Worker thread function that processes tasks from the queue.
- **`processEpollEvents()`**: Main epoll loop for accepting and handling connections.

#### Configuration
- Controlled by `maxWorkers` and `maxTaskQueueSize` from `Config`.

#### Constants
- `DEFAULT_MAX_WORKERS`: 8
- `DEFAULT_MAX_TASK_QUEUE_SIZE`: 100
- `BACKPRESSURE_THRESHOLD_FACTOR`: 2
- `SOCKET_TIMEOUT_SEC`: 10
- `PROCESSING_TIMEOUT_MS`: 5000
- `BACKPRESSURE_DELAY_MS`: 100

---

### `RoktResponseObject.h`

#### Description
Defines the `ResponseObject` class to encapsulate command execution results with a status code, reason, and optional data.

#### Key Methods
- **`getResponse()`**: Returns a formatted string (`status: <code>, reason: <reason>[, datas: <datas>]`) where `datas` is included only for `statusCode = 0`.
- **`hasError()`**: Returns true if `code > 0`.

#### Status Codes
- `0`: OK
- `1`: ERROR
- `503`: Server overloaded
- (See `setDefaultReason()` for full list)

---

### `Config.h` / `Config.cpp`

#### Description
Handles server configuration loading from a JSON file (`config.json`) and environment variables.

#### Structure
- **`Encryption`**: `passphrase`, `iv`
- **`Network`**: `port`, `backlog`
- **`Thread`**: `maxWorkers`, `maxTaskQueueSize`

#### Key Methods
- **`Config(const std::string& filename)`**: Loads configuration with defaults, JSON, and environment overrides.
- **`isValid()`**: Validates configuration parameters.

#### Example `config.json`
```json
{
    "network": { "port": 8080, "backlog": 10 },
    "encryption": { "passphrase": "secret", "iv": "0123456789ABCDEF" },
    "thread": { "maxWorkers": 2, "maxTaskQueueSize": 10 }
}
```

#### Environment Variables
- `ROKT_PORT`, `ROKT_MAX_WORKERS`, `ROKT_MAX_TASK_QUEUE_SIZE`

---

## Test Scripts

### `rokt_load_test.cpp`

#### Description
A load testing script that creates a table (`users`) on the ROKT server and inserts 1 million rows with randomized `name` and `age` values using multiple threads.

#### Key Features
- **Multi-threaded**: Uses `NUM_THREADS` (10) to parallelize insertions.
- **Randomization**: Generates `name` and `age` for each row.
- **Statistics**: Tracks successful/failed inserts, latency, and total execution time.

#### Usage
```bash
g++ -o rokt_load_test rokt_load_test.cpp -pthread
./rokt_load_test
```

#### Example Output
```
--- Statistiques du test ROKT ---
Requêtes totales envoyées : 1000001
Insertions réussies : 1000001
Insertions échouées : 0
Latence moyenne (ms) : 0.58
Temps total d'exécution (ms) : 6250.00
```

---

### `sql_load_test.cpp`

#### Description
A benchmark script using SQLite to create a table (`users`) and insert 1 million rows, mimicking the ROKT test for comparison.

#### Key Features
- **SQLite**: Local database for baseline performance.
- **Multi-threaded**: Similar to ROKT with `NUM_THREADS` (10).
- **Statistics**: Tracks inserts and execution time.

#### Usage
```bash
g++ -o sql_load_test sql_load_test.cpp -lsqlite3 -pthread
./sql_load_test
```

#### Example Output
```
--- Statistiques du test SQLite ---
Requêtes totales envoyées : 1000000
Insertions réussies : 1000000
Insertions échouées : 0
Latence moyenne (ms) : 1.20
```

---

## Building and Running

### Prerequisites
- C++17 compiler (e.g., `g++`)
- SQLite3 library (`libsqlite3-dev` on Ubuntu)
- `nlohmann/json.hpp` for JSON parsing (downloadable from GitHub)

### Compilation
```bash
g++ -o rokt main.cpp SyncService.cpp RoktService.cpp Config.cpp LogService.cpp -pthread
g++ -o rokt_load_test rokt_load_test.cpp -pthread
g++ -o sql_load_test sql_load_test.cpp -lsqlite3 -pthread
```

### Running
1. Start the ROKT server:
   ```bash
   ./rokt
   ```
2. Run the ROKT load test:
   ```bash
   ./rokt_load_test
   ```
3. Run the SQLite benchmark:
   ```bash
   ./sql_load_test
   ```

---

## Performance Notes

- **ROKT**: Fast due to in-memory processing and optimized threading, but actual performance depends on dataset storage (disk vs memory).
- **SQLite**: Slightly slower due to disk I/O, but benefits from indexing and ACID compliance.

---

## Future Improvements
- **Indexing**: Add support for indexes in ROKT to optimize queries (`GET`, `COUNT`).
- **Persistence**: Implement disk storage in ROKT for durability.
- **Extended Commands**: Support more complex SQL-like operations in handlers.