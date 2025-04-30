package handlers

import (
	"net/http"
	"time"

	"mom_gateway/cluster"

	"github.com/gin-gonic/gin"
)

func RegisterHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		var body struct {
			Username string `json:"username"`
			Password string `json:"password"`
		}
		if err := c.ShouldBindJSON(&body); err != nil {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Datos inválidos"})
			return
		}

		ok, msg := cl.RegistrarUsuarioEnTodos(body.Username, body.Password)
		if ok {
			c.JSON(http.StatusOK, gin.H{"mensaje": msg})
		} else {
			c.JSON(http.StatusBadRequest, gin.H{"error": msg})
		}
	}
}

func LoginHandler(cl *cluster.Cluster) gin.HandlerFunc {
	return func(c *gin.Context) {
		var body struct {
			Username string `json:"username"`
			Password string `json:"password"`
		}
		if err := c.ShouldBindJSON(&body); err != nil {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Datos inválidos"})
			return
		}

		ok, token := cl.AutenticarUsuario(body.Username, body.Password)
		if !ok || token == "" {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "Credenciales inválidas"})
			return
		}

		// ⚠️ Estimar 1 hora de expiración (opcional: se puede calcular desde JWT también)
		exp := time.Now().Add(1 * time.Hour).Format("2006-01-02 15:04:05")
		cl.ReplicarTokenEnTodos(body.Username, token, exp)

		c.JSON(http.StatusOK, gin.H{"token": token})
	}
}
