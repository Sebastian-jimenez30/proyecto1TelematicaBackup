package handlers

import (
	"context"
	"net/http"
	"time"

	"mom_gateway/cluster"
	pb "mom_gateway/pb"

	"github.com/gin-gonic/gin"
)

func CrearColaHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		var body struct {
			Nombre string `json:"nombre"`
		}
		if err := c.ShouldBindJSON(&body); err != nil || body.Nombre == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Nombre de cola requerido"})
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
			res, err = nodo.Cliente.CrearCola(ctx, req)
			cancel()
			if err == nil && res.Exito {
				c.JSON(http.StatusOK, gin.H{"mensaje": res.Mensaje})
				return
			}
		}

		c.JSON(http.StatusServiceUnavailable, gin.H{"error": "No se pudo crear la cola. Intenta más tarde"})
	}
}

func EliminarColaHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		nombreCola := c.Param("nombre")
		if nombreCola == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Nombre de cola requerido"})
			return
		}

		token := ExtraerToken(c)
		if token == "" {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "Token requerido"})
			return
		}

		req := &pb.AccionConToken{Token: token, Nombre: nombreCola}
		nodos := cl.NodoResponsableConFallback(nombreCola)

		var res *pb.RespuestaSimple
		var err error

		for _, nodo := range nodos {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			res, err = nodo.Cliente.EliminarCola(ctx, req)
			cancel()
			if err == nil && res.Exito {
				c.JSON(http.StatusOK, gin.H{"mensaje": res.Mensaje})
				return
			}
		}

		c.JSON(http.StatusServiceUnavailable, gin.H{"error": "No se pudo eliminar la cola. Intenta más tarde"})
	}
}

func AutorizarColaHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		nombreCola := c.Param("nombre")
		if nombreCola == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Nombre de cola requerido"})
			return
		}

		var body struct {
			UsuarioObjetivo string `json:"usuario"`
		}
		if err := c.ShouldBindJSON(&body); err != nil || body.UsuarioObjetivo == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Usuario objetivo requerido"})
			return
		}

		token := ExtraerToken(c)
		if token == "" {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "Token requerido"})
			return
		}

		req := &pb.AutorizacionColaRequest{
			Token:           token,
			Nombre:          nombreCola,
			UsuarioObjetivo: body.UsuarioObjetivo,
		}
		nodos := cl.NodoResponsableConFallback(nombreCola)

		var res *pb.RespuestaSimple
		var err error

		for _, nodo := range nodos {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			res, err = nodo.Cliente.AutorizarUsuarioCola(ctx, req)
			cancel()
			if err == nil && res.Exito {

				c.JSON(http.StatusOK, gin.H{"mensaje": res.Mensaje})
				return
			}
		}

		c.JSON(http.StatusServiceUnavailable, gin.H{"error": "No se pudo autorizar al usuario. Intenta más tarde"})
	}
}

func EnviarMensajeColaHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		nombreCola := c.Param("nombre")
		if nombreCola == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Nombre de cola requerido"})
			return
		}

		var body struct {
			Contenido string `json:"contenido"`
		}
		if err := c.ShouldBindJSON(&body); err != nil || body.Contenido == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Contenido requerido"})
			return
		}

		token := ExtraerToken(c)
		if token == "" {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "Token requerido"})
			return
		}

		req := &pb.MensajeConToken{
			Token:     token,
			Nombre:    nombreCola,
			Contenido: body.Contenido,
		}
		nodos := cl.NodoResponsableConFallback(nombreCola)

		var res *pb.RespuestaSimple
		var err error

		for _, nodo := range nodos {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			res, err = nodo.Cliente.EnviarMensajeCola(ctx, req)
			cancel()
			if err == nil && res.Exito {

				c.JSON(http.StatusOK, gin.H{"mensaje": res.Mensaje})
				return
			}
		}

		c.JSON(http.StatusServiceUnavailable, gin.H{"error": "No se pudo enviar el mensaje. Intenta más tarde"})
	}
}

func ConsumirColaHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		nombreCola := c.Param("nombre")
		if nombreCola == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Nombre de cola requerido"})
			return
		}

		token := ExtraerToken(c)
		if token == "" {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "Token requerido"})
			return
		}

		req := &pb.AccionConToken{Token: token, Nombre: nombreCola}
		nodos := cl.NodoResponsableConFallback(nombreCola)

		var res *pb.MensajeTexto
		var err error

		for _, nodo := range nodos {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			res, err = nodo.Cliente.ConsumirMensajeCola(ctx, req)
			cancel()

			if err == nil && res.GetContenido() != "" {
				c.JSON(http.StatusOK, gin.H{
					"remitente": res.Remitente,
					"contenido": res.Contenido,
					"canal":     res.Canal,
					"timestamp": res.Timestamp,
				})
				return
			}
		}

		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "Error al consumir mensaje"})
			return
		}

		c.JSON(http.StatusNoContent, gin.H{})
	}
}

func ListarColasHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		token := ExtraerToken(c)
		if token == "" {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "Token requerido"})
			return
		}

		principal := cl.NodoResponsable("colas")
		replica := cl.NodoSiguiente(principal)

		req := &pb.Token{Token: token}

		ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
		defer cancel()

		res, err := principal.Cliente.ListarColas(ctx, req)
		if err != nil || len(res.GetNombres()) == 0 {
			ctxFallback, cancelFallback := context.WithTimeout(context.Background(), 2*time.Second)
			defer cancelFallback()
			res, err = replica.Cliente.ListarColas(ctxFallback, req)
		}

		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "Error al listar colas"})
			return
		}

		c.JSON(http.StatusOK, gin.H{"colas": res.Nombres})
	}
}
