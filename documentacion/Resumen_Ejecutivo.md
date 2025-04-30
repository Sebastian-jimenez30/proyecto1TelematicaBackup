
## Nombre del Proyecto
**MOM Distribuido (Middleware Orientado a Mensajes)**

## Objetivo General
Desarrollar un sistema distribuido de mensajería que permita a múltiples usuarios autenticados **crear y gestionar colas y tópicos**, enviar y consumir mensajes, y garantizar **persistencia, replicación y tolerancia a fallos** en una arquitectura escalable y moderna.

## Descripción General
Este proyecto implementa un Middleware Orientado a Mensajes (MOM) distribuido, donde las operaciones de mensajería se realizan a través de una arquitectura cliente-servidor compuesta por:

- Un **API Gateway REST** desarrollado en Go (`mom_gateway`), que expone todos los servicios a los clientes mediante HTTP.
- Un **núcleo MOM** desarrollado en C++ (`mom_core_v2`), desplegado en forma de **clúster de tres nodos** independientes que se comunican por gRPC.
- Una base de datos local (`SQLite`) en cada nodo para persistencia.
- Un sistema de autenticación y autorización basado en **tokens JWT**.

## Tecnologías Utilizadas
| Componente         | Tecnología               |
|--------------------|---------------------------|
| API Gateway        | Go (Gin Framework)       |
| Comunicación       | gRPC + Protocol Buffers  |
| Core del Middleware| C++17                    |
| Base de datos      | SQLite                   |
| Seguridad          | JWT + OpenSSL            |
| Distribución       | Replicación + Particionamiento (SHA-1) |

## Componentes Clave
- **Autenticación segura** con JWT (registro y login de usuarios)
- **Gestión de colas y tópicos**, con autorización por creador
- **Envío y consumo de mensajes** con control de acceso
- **Replicación activa en cascada** para alta disponibilidad
- **Particionamiento hash-based** para distribuir cargas entre nodos
- **API REST unificada** para todos los servicios
- **Fallback en lectura** en caso de fallo del nodo principal

## Casos de Uso Típicos
- El usuario se registra y obtiene un token.
- Crea una cola o un tópico, a los cuales solo él puede eliminar.
- Envía mensajes a una cola o publica en un tópico.
- Otros usuarios, si están autorizados o suscritos, pueden consumir mensajes.

## Impacto
Este sistema proporciona una solución práctica, robusta y extensible para manejar flujos de comunicación asincrónicos y desacoplados, comúnmente usados en sistemas distribuidos, microservicios y entornos empresariales que requieren **alta disponibilidad, consistencia eventual y escalabilidad horizontal**.

---
