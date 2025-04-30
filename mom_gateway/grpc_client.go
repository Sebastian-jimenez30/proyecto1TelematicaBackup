package main

import (
	"context"
	"fmt"
	"time"

	pb "mom_gateway/pb"

	"google.golang.org/grpc"
)

var grpcClient pb.MomServiceClient

func InitGRPCClient() error {
	conn, err := grpc.Dial("localhost:50051", grpc.WithInsecure())
	if err != nil {
		return fmt.Errorf("no se pudo conectar con el servidor gRPC: %v", err)
	}
	grpcClient = pb.NewMomServiceClient(conn)
	return nil
}

func Login(username, password string) (string, error) {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	req := &pb.Credenciales{
		Username: username,
		Password: password,
	}

	res, err := grpcClient.AutenticarUsuario(ctx, req)
	if err != nil {
		return "", fmt.Errorf("error al autenticar: %v", err)
	}
	return res.Token, nil
}
