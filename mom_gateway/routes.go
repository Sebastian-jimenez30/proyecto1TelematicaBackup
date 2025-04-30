package main

import (
	"mom_gateway/cluster"
	"mom_gateway/handlers"

	"github.com/gin-gonic/gin"
)

func SetupRoutes(cl *cluster.Cluster) *gin.Engine {
	r := gin.Default()

	// Rutas de autenticación
	r.POST("/register", handlers.RegisterHandler(cl))
	r.POST("/login", handlers.LoginHandler(cl))

	// Rutas de colas
	r.POST("/colas", handlers.CrearColaHandler(cl))
	r.DELETE("/colas/:nombre", handlers.EliminarColaHandler(cl))
	r.POST("/colas/:nombre/autorizar", handlers.AutorizarColaHandler(cl))
	r.POST("/colas/:nombre/enviar", handlers.EnviarMensajeColaHandler(cl))
	r.GET("/colas/:nombre/consumir", handlers.ConsumirColaHandler(cl))
	r.GET("/colas", handlers.ListarColasHandler(cl))

	// Rutas de tópicos
	r.POST("/topicos", handlers.CrearTopicoHandler(cl))
	r.DELETE("/topicos/:nombre", handlers.EliminarTopicoHandler(cl))
	r.POST("/topicos/:nombre/suscribir", handlers.SuscribirseTopicoHandler(cl))
	r.POST("/topicos/:nombre/publicar", handlers.PublicarTopicoHandler(cl))
	r.GET("/topicos/:nombre/consumir", handlers.ConsumirTopicoHandler(cl))
	r.GET("/topicos", handlers.ListarTopicosHandler(cl))

	return r
}
