
#  API REST - Middleware Orientado a Mensajes (MOM)

Este documento describe todos los endpoints REST disponibles para interactuar con el sistema MOM distribuido. Cada operación es delegada al nodo MOM responsable vía gRPC.

---

##  Autenticación

### `POST /register`
**Descripción:** Registra un nuevo usuario en todos los nodos MOM (replicación fuerte).

- **Body:**
```json
{
  "username": "usuario1",
  "password": "1234"
}
```

- **Respuesta:**
```json
{
  "mensaje": "Usuario registrado"
}
```

---

### `POST /login`
**Descripción:** Autentica al usuario en el primer nodo disponible.

- **Body:**
```json
{
  "username": "usuario1",
  "password": "1234"
}
```

- **Respuesta:**
```json
{
  "token": "eyJhbGciOiJIUzI1..."
}
```

---

##  Colas

### `POST /colas`
**Descripción:** Crea una nueva cola. Se replica a otro nodo.

- **Header:** `Authorization: Bearer <token>`
- **Body:**
```json
{
  "nombre": "cola1"
}
```

---

### `DELETE /colas/:nombre`
**Descripción:** Elimina una cola si el usuario autenticado es el creador.

- **Header:** `Authorization: Bearer <token>`

---

### `POST /colas/:nombre/autorizar`
**Descripción:** Autoriza a otro usuario para consumir mensajes de la cola.

- **Header:** `Authorization: Bearer <token>`
- **Body:**
```json
{
  "usuario": "juan123"
}
```

---

### `POST /colas/:nombre/enviar`
**Descripción:** Envia un mensaje a la cola. Se replica a nodo siguiente.

- **Header:** `Authorization: Bearer <token>`
- **Body:**
```json
{
  "contenido": "Mensaje de prueba"
}
```

---

### `GET /colas/:nombre/consumir`
**Descripción:** Consume un mensaje desde la cola si el usuario está autorizado.

- **Header:** `Authorization: Bearer <token>`
- **Respuesta:**
```json
{
  "remitente": "usuario1",
  "contenido": "Mensaje de prueba",
  "canal": "cola1",
  "timestamp": "2024-04-25 15:00:00"
}
```

---

### `GET /colas`
**Descripción:** Lista todas las colas creadas por el usuario autenticado.

- **Header:** `Authorization: Bearer <token>`
- **Respuesta:**
```json
{
  "colas": ["cola1", "tareas", "alertas"]
}
```

---

##  Tópicos

### `POST /topicos`
**Descripción:** Crea un nuevo tópico (se replica).

- **Header:** `Authorization: Bearer <token>`
- **Body:**
```json
{
  "nombre": "noticias"
}
```

---

### `DELETE /topicos/:nombre`
**Descripción:** Elimina un tópico si el usuario es el creador.

- **Header:** `Authorization: Bearer <token>`

---

### `POST /topicos/:nombre/suscribir`
**Descripción:** Se suscribe al tópico especificado.

- **Header:** `Authorization: Bearer <token>`

---

### `POST /topicos/:nombre/publicar`
**Descripción:** Publica un mensaje en el tópico (replicado).

- **Header:** `Authorization: Bearer <token>`
- **Body:**
```json
{
  "contenido": "¡Bienvenidos al canal!"
}
```

---

### `GET /topicos/:nombre/consumir`
**Descripción:** Consume mensajes pendientes del tópico.

- **Header:** `Authorization: Bearer <token>`
- **Respuesta:**
```json
{
  "mensajes": [
    {
      "remitente": "admin",
      "contenido": "¡Bienvenidos al canal!",
      "canal": "noticias",
      "timestamp": "2024-04-25 12:00:00"
    },
    ...
  ]
}
```

---

### `GET /topicos`
**Descripción:** Lista todos los tópicos a los que el usuario está suscrito.

- **Header:** `Authorization: Bearer <token>`
- **Respuesta:**
```json
{
  "topicos": ["noticias", "eventos", "anuncios"]
}
```

---

##  Errores comunes

- `401 Unauthorized`: Token ausente o inválido
- `403 Forbidden`: Acción no permitida (no eres el creador)
- `400 Bad Request`: Faltan datos requeridos
- `500 Internal Server Error`: Falla de red o nodo caído

---

##  Notas

- Todas las rutas que modifican estado utilizan **replicación activa**.
- Las rutas de lectura siempre consultan al **nodo principal** (por hash).
- El token se pasa por header estándar `Authorization: Bearer <token>`.
- Se recomienda usar **Postman** o **curl** para probar la API REST.
