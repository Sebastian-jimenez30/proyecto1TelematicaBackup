package handlers

import (
	"context"
	"net/http"
	"time"

	"mom_gateway/cluster"
	pb "mom_gateway/pb"

	"github.com/gin-gonic/gin"
)

func CrearTopicoHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		var body struct {
			Nombre string `json:"nombre"`
		}
		if err := c.ShouldBindJSON(&body); err != nil || body.Nombre == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Nombre de tópico requerido"})
			return
		}

		token := ExtraerToken(c)
		if token == "" {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "Token requerido"})
			return
		}

		req := &pb.AccionConToken{Token: token, Nombre: body.Nombre}
		nodos := cl.NodoResponsableConFallback(body.Nombre)

		var res *pb.RespuestaSimple
		var err error

		for _, nodo := range nodos {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			res, err = nodo.Cliente.CrearTopico(ctx, req)
			cancel()
			if err == nil && res.Exito {
				c.JSON(http.StatusOK, gin.H{"mensaje": res.Mensaje})
				return
			}
		}

		c.JSON(http.StatusServiceUnavailable, gin.H{"error": "No se pudo crear el tópico. Intenta más tarde"})
	}
}

func EliminarTopicoHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		nombre := c.Param("nombre")
		token := ExtraerToken(c)
		if nombre == "" || token == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Parámetros incompletos"})
			return
		}

		req := &pb.AccionConToken{Token: token, Nombre: nombre}
		nodos := cl.NodoResponsableConFallback(nombre)

		var res *pb.RespuestaSimple
		var err error

		for _, nodo := range nodos {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			res, err = nodo.Cliente.EliminarTopico(ctx, req)
			cancel()
			if err == nil && res.Exito {
				c.JSON(http.StatusOK, gin.H{"mensaje": res.Mensaje})
				return
			}
		}

		c.JSON(http.StatusServiceUnavailable, gin.H{"error": "No se pudo eliminar el tópico. Intenta más tarde"})
	}
}

func SuscribirseTopicoHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		nombre := c.Param("nombre")
		token := ExtraerToken(c)
		if nombre == "" || token == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Parámetros incompletos"})
			return
		}

		req := &pb.AccionConToken{Token: token, Nombre: nombre}
		nodos := cl.NodoResponsableConFallback(nombre)

		var res *pb.RespuestaSimple
		var err error

		for _, nodo := range nodos {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			res, err = nodo.Cliente.SuscribirseTopico(ctx, req)
			cancel()
			if err == nil && res.Exito {
				c.JSON(http.StatusOK, gin.H{"mensaje": res.Mensaje})
				return
			}
		}

		c.JSON(http.StatusServiceUnavailable, gin.H{"error": "No se pudo suscribir al tópico. Intenta más tarde"})
	}
}

func PublicarTopicoHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		nombre := c.Param("nombre")
		token := ExtraerToken(c)

		var body struct {
			Contenido string `json:"contenido"`
		}
		if err := c.ShouldBindJSON(&body); err != nil || nombre == "" || body.Contenido == "" || token == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Datos incompletos"})
			return
		}

		req := &pb.MensajeConToken{
			Token:     token,
			Nombre:    nombre,
			Contenido: body.Contenido,
		}
		nodos := cl.NodoResponsableConFallback(nombre)

		var res *pb.RespuestaSimple
		var err error

		for _, nodo := range nodos {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			res, err = nodo.Cliente.PublicarEnTopico(ctx, req)
			cancel()
			if err == nil && res.Exito {
				c.JSON(http.StatusOK, gin.H{"mensaje": res.Mensaje})
				return
			}
		}

		c.JSON(http.StatusServiceUnavailable, gin.H{"error": "No se pudo publicar el mensaje. Intenta más tarde"})
	}
}

func ConsumirTopicoHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		nombre := c.Param("nombre")
		token := ExtraerToken(c)
		if nombre == "" || token == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Datos incompletos"})
			return
		}

		req := &pb.AccionConToken{Token: token, Nombre: nombre}
		nodos := cl.NodoResponsableConFallback(nombre)

		var res *pb.ListaMensajes
		var err error

		for _, nodo := range nodos {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			res, err = nodo.Cliente.ConsumirDesdeTopico(ctx, req)
			cancel()
			if err == nil && len(res.Mensajes) > 0 {
				break
			}
		}

		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "Error al consumir mensajes"})
			return
		}

		var mensajes []gin.H
		for _, m := range res.Mensajes {
			mensajes = append(mensajes, gin.H{
				"remitente": m.Remitente,
				"contenido": m.Contenido,
				"canal":     m.Canal,
				"timestamp": m.Timestamp,
			})
		}

		c.JSON(http.StatusOK, gin.H{"mensajes": mensajes})
	}
}

func ListarTopicosHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		token := ExtraerToken(c)
		if token == "" {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "Token requerido"})
			return
		}

		req := &pb.Token{Token: token}
		nodos := cl.NodoResponsableConFallback("topicos")

		var res *pb.ListaNombres
		var err error

		for _, nodo := range nodos {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			res, err = nodo.Cliente.ListarTopicos(ctx, req)
			cancel()
			if err == nil && len(res.Nombres) > 0 {
				break
			}
		}

		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "Error al listar tópicos"})
			return
		}

		c.JSON(http.StatusOK, gin.H{"topicos": res.Nombres})
	}
}
