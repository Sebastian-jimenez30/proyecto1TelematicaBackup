
#  Middleware Orientado a Mensajes (MOM) Distribuido

Este proyecto implementa un sistema **Middleware Orientado a Mensajes (MOM)** distribuido, con soporte para colas y tópicos, múltiples nodos, replicación, autenticación, y un API REST para interacción con clientes.

---

##  Tabla de Contenidos

- [ Requerimientos](#-requerimientos)
- [ Análisis del Problema](#-análisis-del-problema)
- [ Diseño del Sistema](#️-diseño-del-sistema)
- [ Implementación](#️-implementación)
- [ Uso y Aplicación](#-uso-y-aplicación)
- [ Documentación Adicional](#-documentación-adicional)

---

##  Requerimientos

### Funcionales

- Autenticación de usuarios (registro, login, verificación de tokens)
- Creación, eliminación y autorización de acceso a **colas**
- Envío y consumo de mensajes en colas (FIFO)
- Creación, eliminación y suscripción a **tópicos**
- Publicación y consumo de mensajes desde tópicos (con offsets por usuario)
- Listado de colas y tópicos
- Persistencia de datos en SQLite
- Exposición de servicios a través de un API REST

### No Funcionales

- Replicación de operaciones entre nodos (estrategia activa en cascada)
- Particionamiento determinista de colas y tópicos
- Tolerancia a fallos (replicas secundarias)
- Seguridad básica con JWT (firma y expiración)
- Escalabilidad horizontal (soporte para múltiples nodos)
- Transparencia al cliente REST (el gateway oculta los nodos)

---

##  Análisis del Problema

La comunicación asincrónica y desacoplada entre componentes de software requiere un intermediario confiable: un **Middleware Orientado a Mensajes (MOM)**.

### Necesidades identificadas

- Necesidad de una solución distribuida para manejar fallos
- Requerimiento de almacenamiento persistente y ordenado
- Diferentes patrones de comunicación: **Cola** (punto a punto), **Tópico** (publicador/subscriptor)
- Control de acceso por usuario
- Necesidad de consumo bajo demanda (**modo pull**)

---

##  Diseño del Sistema

### Arquitectura General

- **MOM Core (C++)**: lógica de negocio y persistencia
- **API Gateway (Go)**: expone servicios REST y se comunica con los MOMs vía gRPC
- **Clúster MOM**: conjunto de nodos replicados y particionados

### Modelo C4

- Nivel 1: Sistema de mensajería distribuido (usuarios ↔ gateway ↔ nodos MOM)
- Nivel 2: Componentes como `broker`, `usuario`, `cola`, `topico`, `persistencia`
- Nivel 3: Diagrama de clases UML para modelar entidades y relaciones

### Particionamiento

- Hash SHA-1 del nombre de canal determina el nodo responsable.

### Replicación

- Todas las operaciones mutables (crear, eliminar, autorizar, publicar) se replican en al menos **un nodo adicional**.

### Seguridad

- Uso de tokens JWT firmados y con expiración
- Verificación del token en cada operación
- Autorización por creador para eliminar o modificar

---

## 🛠 Implementación

### Tecnologías Usadas

- C++17 + gRPC + SQLite (MOM Core)
- Go + Gin + gRPC client (API Gateway)
- Protocol Buffers (definición de contratos)


### Componentes Clave

- `Broker`: fachada del MOM
- `Cola`, `Topico`: entidades y su lógica
- `Persistencia`: conexión y operaciones SQLite
- `Usuario`: autenticación y verificación de JWT
- `cluster.go`: enrutamiento y replicación
- `handlers/*.go`: endpoint handlers REST

---

##  Uso y Aplicación

### Ejecución

#### 1. Iniciar 3 nodos MOM:

En terminales distintas ejecutar los siguientes comando 1 veces por cada servidor mom

```bash
cd mom
./run_dev.sh 0
```
Debe esperar a ver la dirección proporcionada por el servidor, por ejemplo:

```bash
Servidor MOM escuchando en 192.168.1.14:50051
```

#### 2. Ejecutar el API Gateway:

```bash
cd mom_gateway
go run . $dirMom1 $dirMom2 $dirMom3
```

ejemplo

```bash
cd mom_gateway
go run . 192.168.1.14:50051 192.168.1.14:50052 192.168.1.14:50051
```

#### 3.1. Acceder Via Cliente

```bash
cd client
./client.py
```
El ejecutable le mostrará

```bash
=== Menú Principal ===
1. Registrar usuario
2. Iniciar sesión
3. Crear cola
4. Eliminar cola
5. Autorizar usuario en cola
6. Enviar mensaje a cola
7. Consumir mensaje de cola
8. Listar colas
9. Crear tópico
10. Eliminar tópico
11. Suscribirse a tópico
12. Publicar mensaje en tópico
13. Consumir mensajes de tópico
14. Listar tópicos
15. Salir
Selecciona una opción: 
```

Aquí podra operar de una manera sencilla las peticiones

#### 3.2. Acceder vía Postman o curl a:

```
http://localhost:8080
```

### Flujo de Usuario

1. Registrar usuario
2. Login → token
3. Crear cola o tópico
4. Enviar o publicar mensaje
5. Consumir desde cola o tópico
6. Eliminar o autorizar acceso

### Ejemplo de curl

```bash
curl -X POST http://localhost:8080/register \
  -H "Content-Type: application/json" \
  -d '{"username":"sebastian","password":"1234"}'

curl -X POST http://localhost:8080/login \
  -d '{"username":"sebastian","password":"1234"}'
```

---

##  Autores

- Sebastián Jimenez
- Carlos Alberto Mazo Gil
- Juan Jose Restrepo Hernandez

---

##  Licencia

Este proyecto es parte del curso ST0263 - Universidad EAFIT - 2025.
