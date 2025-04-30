


#  Particionamiento y Replicación en el Middleware Orientado a Mensajes (MOM)

## 1. Introducción

El sistema MOM distribuido implementa una arquitectura basada en múltiples nodos MOM que cooperan para ofrecer disponibilidad, escalabilidad y tolerancia a fallos. Este documento detalla las estrategias técnicas aplicadas para **particionar** recursos y **replicar** operaciones críticas, incluyendo los algoritmos de hashing utilizados, el manejo de fallos, la estructura de datos y la lógica de replicación activa.

---

## 2. Objetivo

Garantizar:
- **Distribución balanceada** de las colas y tópicos entre los nodos.
- **Replicación activa** de operaciones críticas en nodos secundarios.
- **Tolerancia a fallos** en caso de caída parcial del clúster.
- **Escalabilidad horizontal** a través de un esquema extensible.

---

## 3. Particionamiento

### 3.1. ¿Qué se particiona?

Se particionan por nombre:
- **Colas**
- **Tópicos**

Cada uno es asignado a un nodo específico del clúster.

### 3.2. Algoritmo de Hashing

Se utiliza una función hash determinística para obtener el nodo responsable:

```go
hash := sha1.Sum([]byte(nombre))
idx := int(hash[0]) % len(Nodos)
return Nodos[idx]
```

### 3.3. Detalles:

- Se emplea **SHA-1** por su velocidad y dispersión suficiente para nombres cortos.
- Solo se toma el **primer byte del hash**, lo cual genera un índice de 0 a 255.
- Ese índice se mapea a un nodo `n` entre los `len(Nodos)` disponibles mediante `mod`.

#### Ventajas:

Rápido y determinístico  
Distribución equitativa (aunque no perfecta)  
Bajo costo computacional

#### Limitación:

 No soporta **reconfiguración sin redistribución** (como Consistent Hashing)  
 Requiere mejoras si se agregan nodos dinámicamente

---

## 4. Replicación

### 4.1. ¿Qué se replica?

Se replican todas las **operaciones críticas con estado**:

| Tipo           | Operación                                 | Replica       |
|----------------|-------------------------------------------|---------------|
| Usuario        | Registro, Token login                     | En los 3 nodos |
| Cola           | Crear, Eliminar, Autorizar, Enviar        | Nodo + siguiente |
| Tópico         | Crear, Eliminar, Suscribir, Publicar      | Nodo + siguiente |
| Mensajes       | Encolados o publicados                    | Nodo + siguiente |
| Autorizaciones | Relación usuario-cola                     | Nodo + siguiente |
| Suscripciones  | Relación usuario-tópico                   | Nodo + siguiente |

### 4.2. Esquema de replicación

#### Estrategia: **Replicación Activa en Cascada (N + 1)**

```go
go cl.ReplicarEnNodosSiguientes(nombre, func(client MomServiceClient) error {
    _, err := client.CrearCola(ctx, req)
    return err
})
```

- Se replica al **siguiente nodo circular** (`NodoSiguiente(nodoResponsable)`).
- La replicación es **asíncrona** (con `go`) para evitar bloqueo en la respuesta REST.
- Se replican también los errores para logueo, pero no afectan al cliente si el nodo principal fue exitoso.

### 4.3. Función utilizada

```go
func (c *Cluster) ReplicarEnNodosSiguientes(nombre string, accion func(pb.MomServiceClient) error) {
    nodo := c.NodoResponsable(nombre)
    for i := 1; i < len(c.Nodos); i++ {
        siguiente := c.Nodos[(index + i) % len(c.Nodos)]
        go accion(siguiente.Cliente)
    }
}
```

- Replicación en **todos los nodos excepto el principal** (puede limitarse a uno).
- Se usa una **goroutine por nodo** para evitar bloqueo.

---

## 5. Recuperación ante fallos

### 5.1. Fallo del nodo principal

- Se puede redirigir la operación a su réplica inmediatamente o reintentar.
- En operaciones de lectura, esto aún **no está implementado completamente**.

### 5.2. Fallo parcial

- Gracias a la replicación, los datos siguen accesibles si un nodo cae.
- Las operaciones críticas persisten al menos en dos nodos.

---

## 6. Diseño distribuido

- Comunicación entre servicios: gRPC
- Coordinación: API Gateway
- Replicación y hashing: `cluster.go`
- Idempotencia: Cada operación de escritura puede ejecutarse múltiples veces sin efectos colaterales graves (por diseño).

---

## 7. Escalabilidad

- Se puede escalar horizontalmente replicando el mismo sistema con `n` MOMs.
- Idealmente, se debe sustituir el hash simple por **Consistent Hashing con Virtual Nodes** en un entorno más grande.

---

## 8. Alternativas evaluadas

| Estrategia               | Resultado               |
|--------------------------|-------------------------|
| Hash aleatorio           |  No determinístico    |
| Round Robin por cliente  |  No mantiene afinidad |
| Consistent Hashing       |  Mejor distribución, pero mayor complejidad |
| SHA-1 simple             |  Aceptable para 3 nodos actuales |

---

## 9. Conclusión

El sistema implementa un esquema de particionamiento por hashing simple y una replicación activa hacia al menos un nodo secundario. Esta combinación permite alta disponibilidad y una base sólida para escalar el sistema en producción. Las operaciones críticas se sincronizan entre nodos y se identifican por su nombre, lo que permite trazabilidad y control distribuido sin depender de un único punto de falla.

