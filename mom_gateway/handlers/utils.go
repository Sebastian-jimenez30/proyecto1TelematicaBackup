package handlers

import (
	"strings"

	"github.com/gin-gonic/gin"
)

func ExtraerToken(c *gin.Context) string {
	auth := c.GetHeader("Authorization")
	if strings.HasPrefix(auth, "Bearer ") {
		return strings.TrimPrefix(auth, "Bearer ")
	}
	return ""
}
