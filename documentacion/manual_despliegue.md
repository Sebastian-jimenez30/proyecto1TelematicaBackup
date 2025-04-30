#  Manual de Despliegue - Middleware Orientado a Mensajes (MOM)

Este documento describe paso a paso cómo desplegar e iniciar el sistema MOM distribuido, incluyendo su servidor gRPC escrito en C++ y el API Gateway REST implementado en Go.

---

##  Requisitos de sistema

### Sistema Operativo
- Linux (Ubuntu recomendado, también compatible con WSL2)

### Requisitos generales
- CMake >= 3.10
- Make
- Compilador C++17 (`g++`)
- Git
- `protoc` (Protocol Buffers Compiler >= 3.20)
- Plugin gRPC para `protoc`
- Go >= 1.20

---

##  Dependencias C++

1. Instala **Protobuf y gRPC**:

```bash
sudo apt update && sudo apt install -y build-essential autoconf libtool pkg-config \
  libprotobuf-dev protobuf-compiler protobuf-compiler-grpc \
  libgrpc++-dev libssl-dev sqlite3 libsqlite3-dev
```

2. Verifica que `protoc` y `grpc_cpp_plugin` están disponibles:

```bash
which protoc
which grpc_cpp_plugin
```

---

##  Estructura del proyecto

```
mom_core/
├── CMakeLists.txt
├── build/
├── proto/
│   └── mom.proto
├── src/
│   ├── mom_server.cpp
│   ├── *.cpp
├── include/
│   ├── *.hpp
├── data/
│   └── mom.db
```

---

##  Compilar los servidores MOM (C++ gRPC)

### 1. Generar archivos gRPC desde .proto:

```bash
cd mom_core
protoc -I proto --cpp_out=src --grpc_out=src --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) proto/mom.proto
```

### 2. Compilar con CMake:

```bash
mkdir -p build
cd build
cmake ..
make grpc_server
```

Esto generará el ejecutable `grpc_server` en `build/`.

---

##  Iniciar los nodos MOM

Cada nodo se debe ejecutar en un puerto diferente y con su propia base de datos. Ejecuta cada uno en terminales separados:

```bash
# Terminal 1 (mom1)
./grpc_server ../data/mom1.db 50051

# Terminal 2 (mom2)
./grpc_server ../data/mom2.db 50052

# Terminal 3 (mom3)
./grpc_server ../data/mom3.db 50053
```

---

##  API Gateway (Go)

### 1. Instalar dependencias Go

```bash
cd mom_gateway
go mod tidy
```

Asegúrate que los archivos generados por `protoc` estén en `mom_gateway/pb`.

---

### 2. Ejecutar el Gateway

```bash
go run main.go
```

Esto inicia el servidor REST en:

```
http://localhost:8080
```

---

##  Probar desde Postman o curl

- Registra un usuario:
```http
POST http://localhost:8080/register
Body: { "username": "sebastian", "password": "1234" }
```

- Login:
```http
POST http://localhost:8080/login
```

- Crear cola:
```http
POST http://localhost:8080/colas
Header: Authorization: Bearer <token>
Body: { "nombre": "tareas" }
```

---

##  Notas adicionales

- El puerto `50051` es el nodo MOM1, `50052` MOM2, y `50053` MOM3.
- Puedes modificar el número de nodos desde `cluster.go`.
- Para regenerar los archivos `.pb.cc` y `.grpc.pb.cc`, usa el comando `protoc` mencionado arriba.

---

##  Verificación

Verifica que:
- Todos los nodos están corriendo sin errores.
- El API Gateway responde en `http://localhost:8080`.
- Los registros y tokens son replicados en los tres MOMs.

---

##  Archivos generados

- `mom.pb.h`, `mom.grpc.pb.h`, `mom.pb.cc`, `mom.grpc.pb.cc` generados automáticamente con `protoc`.

---