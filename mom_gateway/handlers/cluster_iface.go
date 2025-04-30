package handlers

import (
	"mom_gateway/cluster" // Importar el tipo Nodo desde el paquete cluster
	pb "mom_gateway/pb"
)

type Cluster interface {
	SeleccionarPrimario(nombre string) cluster.Nodo
	SeleccionarSecundario(primario cluster.Nodo) cluster.Nodo
	GetCliente(id string) pb.MomServiceClient
}
