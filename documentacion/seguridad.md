#  Documento de Seguridad del Sistema MOM Distribuido

## 1. Introducción

Este documento detalla las medidas de seguridad implementadas en el Middleware Orientado a Mensajes (MOM) distribuido, incluyendo autenticación, control de acceso, encriptación, gestión de tokens y consideraciones adicionales para proteger la integridad, confidencialidad y disponibilidad del sistema.

---

## 2. Autenticación y Autorización

### 2.1. Autenticación con JWT

- El sistema utiliza **JSON Web Tokens (JWT)** para autenticar usuarios.
- Al iniciar sesión, el servidor genera un JWT firmado con una clave privada.
- Este token contiene:
  - `username`
  - `exp` (fecha de expiración)
- El cliente debe incluir el token en la cabecera `Authorization` para futuras solicitudes.

### 2.2. Expiración de Sesiones

- Los tokens tienen una duración limitada (ej. 1 hora).
- Esta expiración se valida tanto en el cliente como en la base de datos.
- Los tokens expirados son automáticamente rechazados.

### 2.3. Almacenamiento Seguro de Tokens

- Cada token generado se almacena en la base de datos junto con su fecha de expiración.
- Al validar un token, se comprueba que esté activo en la base de datos.

### 2.4. Autorización basada en roles

- Cada recurso está protegido:
  - **Colas**: solo el creador puede eliminarla o autorizar consumidores.
  - **Tópicos**: solo el creador puede publicar o eliminar.
- Los permisos se verifican por usuario mediante relaciones en las tablas `autorizaciones` y `suscripciones`.

---

## 3. Seguridad en la Comunicación

### 3.1. Canales de Comunicación

- **API Gateway ←→ Cliente (Frontend / Postman)**:
  - Utiliza **HTTP** (con opción a habilitar HTTPS en despliegue).
- **API Gateway ←→ MOM Nodes**:
  - Comunicación vía **gRPC**, con capacidad para usar **TLS**.

### 3.2. Cifrado en tránsito (opcional)

- Para entornos productivos, se puede habilitar **TLS** en:
  - el gateway Go (`grpc.WithTransportCredentials`)
  - el servidor C++ (`grpc::SslServerCredentialsOptions`)

---

## 4. Protección contra amenazas comunes

| Amenaza                        | Medida Implementada                                             |
|-------------------------------|-----------------------------------------------------------------|
| **Falsificación de identidad**| Validación de tokens JWT firmados y verificación en la base de datos. |
| **Intercepción de datos**     | Posibilidad de habilitar TLS para cifrado extremo a extremo.   |
| **Reutilización de tokens**   | Control de expiración en base de datos.                        |
| **Acceso no autorizado**      | Control de permisos a nivel de usuario y recurso.              |
| **Inyección SQL**             | Uso exclusivo de SQLite con sentencias preparadas y validadas. |
| **Errores silenciosos**       | Registro de logs de errores en consola (opcional: persistirlos).|

---

## 5. Tolerancia a Fallos y Resiliencia

- Replicación activa: los mensajes y recursos se replican en al menos dos nodos.
- Si un nodo falla, las operaciones de lectura intentan reintentar con otro nodo (estrategia en progreso).
- Se puede escalar a un entorno multi-host con load balancing.

---

## 6. Buenas prácticas recomendadas

- **Rotación de claves JWT** periódica (en despliegue productivo).
- **Registros de logs sensibles cifrados** (si se implementa almacenamiento en disco).
- **Reintentos de conexión gRPC** en caso de caída temporal del nodo.
- **Control de concurrencia** en la base de datos (SQLite tiene locking por archivo, válido en uso actual).

---

## 7. Seguridad en el Código

- El sistema está modularizado, evitando fugas de lógica entre capas.
- Uso de `context.Context` para limitar tiempos de ejecución y evitar operaciones colgadas.
- Todas las entradas del usuario son validadas en los handlers REST.

---

## 8. Consideraciones Futuras

- Integrar autenticación externa (OAuth2, LDAP).
- Implementar **reintentos inteligentes** y auto-switchover entre nodos en caso de caída.
- Cifrado en reposo para mensajes sensibles (actualmente plaintext en SQLite).
- Alertas y monitoreo de seguridad.

---

## 9. Conclusión

El sistema implementa múltiples capas de seguridad que protegen los datos del usuario, la autenticación y la integridad del sistema en un entorno distribuido. Si bien se han tomado medidas sólidas, la arquitectura es extensible y permite mejoras progresivas en función de los requerimientos del entorno de despliegue.

