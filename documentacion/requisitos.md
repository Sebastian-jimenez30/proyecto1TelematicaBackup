# Especificación de Requisitos

##  Requisitos Funcionales

| Código | Requisito Funcional                                                                 |
|--------|--------------------------------------------------------------------------------------|
| RF01   | El sistema debe permitir a los usuarios registrarse con nombre de usuario y contraseña. |
| RF02   | El sistema debe autenticar a los usuarios mediante credenciales y generar un token JWT. |
| RF03   | Solo los usuarios autenticados podrán acceder al sistema.                            |
| RF04   | El sistema debe permitir a un usuario crear colas y tópicos con nombres únicos.      |
| RF05   | Solo el creador de una cola o tópico podrá eliminarla.                               |
| RF06   | El creador de una cola podrá autorizar a otros usuarios para consumir mensajes de ella. |
| RF07   | El sistema debe permitir publicar mensajes en tópicos o enviar mensajes a colas.     |
| RF08   | El sistema debe permitir a los usuarios consumir mensajes desde colas y tópicos.     |
| RF09   | Los mensajes deben ser almacenados de forma persistente con sus metadatos (remitente, timestamp, etc.). |
| RF10   | El sistema debe replicar las operaciones de escritura en al menos otro nodo del clúster. |
| RF11   | El sistema debe manejar suscripciones a tópicos, y solo los usuarios suscritos podrán consumir mensajes. |
| RF12   | El sistema debe exponer todos sus servicios a través de un API REST.                 |
| RF13   | El sistema debe incluir un mecanismo para listar colas y tópicos visibles para el usuario autenticado. |

---

##  Requisitos No Funcionales

| Código | Requisito No Funcional                                                               |
|--------|----------------------------------------------------------------------------------------|
| RNF01  | El sistema debe ser accesible a través de HTTP en el puerto 8080 del API Gateway.     |
| RNF02  | Las comunicaciones entre componentes (Gateway y MOMs) deben realizarse mediante gRPC. |
| RNF03  | La información del usuario y los tokens deben almacenarse de manera segura.           |
| RNF04  | El sistema debe usar JWT firmado digitalmente como método de autenticación.           |
| RNF05  | La base de datos utilizada debe ser liviana, embebida y soportar múltiples nodos (SQLite). |
| RNF06  | El sistema debe ser tolerante a fallos: si un nodo principal falla, otro debe atender la solicitud. |
| RNF07  | El sistema debe estar diseñado para soportar escalabilidad horizontal.                |
| RNF08  | La replicación debe ser en cascada para reducir el impacto en la latencia.            |
| RNF09  | El sistema debe implementar consistencia eventual entre los nodos.                    |
| RNF10  | El sistema debe tener logs de operación para seguimiento y trazabilidad.              |

---

