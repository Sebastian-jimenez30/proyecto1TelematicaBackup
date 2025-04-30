package main

import (
	"fmt"
	"mom_gateway/cluster"
	"net"
	"os"
)

func getLocalIP() string {
	addrs, err := net.InterfaceAddrs()
	if err != nil {
		return "localhost"
	}

	for _, addr := range addrs {

		if ipNet, ok := addr.(*net.IPNet); ok && !ipNet.IP.IsLoopback() {
			if ipNet.IP.To4() != nil {
				return ipNet.IP.String()
			}
		}
	}

	return "localhost"
}

func main() {
	if len(os.Args) < 4 {
		fmt.Println("Uso: mom_gateway <nodo1> <nodo2> <nodo3>")
		return
	}

	nodo1 := os.Args[1]
	nodo2 := os.Args[2]
	nodo3 := os.Args[3]

	fmt.Println("🔌 Inicializando conexiones con los nodos MOM vía gRPC...")
	cl := cluster.NuevoCluster(nodo1, nodo2, nodo3)

	localIP := getLocalIP()
	fmt.Printf("🌐 Servidor REST escuchando en http://%s:8080\n", localIP)
	router := SetupRoutes(cl)
	router.Run(fmt.Sprintf("%s:8080", localIP))
}
