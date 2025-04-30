
This directory contains the core Message-Oriented Middleware logic implemented in C++. It handles local message processing, storage, replication, and inter-node communication.

## Contents
- `src/`: Main application logic for MOM node lifecycle.
- `grpc_server/`: Implements gRPC server to accept calls from other nodes or the Go API.
- `messaging/`: Queue and topic management, message lifecycle, and delivery logic.
- `replication/`: Logic for sending and receiving replicated messages across nodes.
- `persistence/`: SQLite or JSON file-based local storage for messages and system state.
- `utils/`: Helper functions (e.g., hashing, logging, JSON parsing).
- `proto/`: Shared `.proto` files (same as in `go-api/proto`) to generate C++ bindings.

Each MOM node runs independently and collaborates with others to ensure distributed consistency and reliability.


