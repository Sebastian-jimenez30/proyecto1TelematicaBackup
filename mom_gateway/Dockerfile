# Imagen base con Go
FROM golang:1.24.2

# Crear directorio de trabajo dentro del contenedor
WORKDIR /app

# Copiar los archivos del proyecto
COPY . .

# Descargar dependencias y compilar binario
RUN go mod download
RUN go build -o mom_gateway main.go routes.go grpc_client.go

# Exponer puerto HTTP REST
EXPOSE 8080

# Comando por defecto al iniciar (los nodos se pasan como argumentos al hacer run)
ENTRYPOINT ["/app/mom_gateway"]
