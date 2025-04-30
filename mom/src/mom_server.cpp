#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "mom.grpc.pb.h"
#include "broker.hpp"
#include "NodoManager.hpp"
#include <thread>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <filesystem>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class MomServiceImpl final : public mom::MomService::Service {
private:
    Broker& broker;
    std::string nodoActual;

public:
    MomServiceImpl(Broker& b, const std::string& nodoActual_) : broker(b), nodoActual(nodoActual_) {}

    // ========== Servicios de Usuario ==========
    Status RegistrarUsuario(ServerContext*, const mom::Credenciales* req, mom::RespuestaSimple* res) override {
        bool ok = broker.registrarUsuario(req->username(), req->password());
        res->set_exito(ok);
        res->set_mensaje(ok ? "Usuario registrado" : "Error al registrar");
        return Status::OK;
    }

    Status AutenticarUsuario(ServerContext*, const mom::Credenciales* req, mom::Token* res) override {
        std::string token;
        bool ok = broker.autenticarUsuario(req->username(), req->password(), token);
        if (ok) res->set_token(token);
        return Status::OK;
    }

    Status GuardarTokenReplica(ServerContext*, const mom::TokenConExpiracion* req, mom::RespuestaSimple* res) override {
        bool ok = broker.guardarTokenReplica(req->username(), req->token(), req->expiracion());
        res->set_exito(ok);
        res->set_mensaje(ok ? "Token replicado" : "Error");
        return Status::OK;
    }

    // ========== Servicios de Cola ==========
    Status CrearCola(ServerContext*, const mom::AccionConToken* req, mom::RespuestaSimple* res) override {
        std::cout << "[CrearCola] Solicitud recibida para crear cola: " << req->nombre() << " por token: " << req->token() << "\n";
    
        bool ok = broker.crearColaDesdeCliente(req->nombre(), req->token());
    
        std::cout << (ok ? "[CrearCola] Cola creada exitosamente.\n" : "[CrearCola] Error al crear cola.\n");
    
        res->set_exito(ok);
        res->set_mensaje(ok ? "Cola creada" : "Error al crear cola");
        return Status::OK;
    }

    Status EliminarCola(ServerContext*, const mom::AccionConToken* req, mom::RespuestaSimple* res) override {
        bool ok = broker.eliminarColaDesdeCliente(req->nombre(), req->token());
        res->set_exito(ok);
        res->set_mensaje(ok ? "Cola eliminada" : "Error al eliminar cola");
        return Status::OK;
    }

    Status AutorizarUsuarioCola(ServerContext*, const mom::AutorizacionColaRequest* req, mom::RespuestaSimple* res) override {
        bool ok = broker.autorizarColaDesdeCliente(req->nombre(), req->usuarioobjetivo(), req->token());
        res->set_exito(ok);
        res->set_mensaje(ok ? "Usuario autorizado" : "Error al autorizar");
        return Status::OK;
    }

    Status EnviarMensajeCola(ServerContext*, const mom::MensajeConToken* req, mom::RespuestaSimple* res) override {
        bool ok = broker.enviarMensajeAColaDesdeCliente(req->nombre(), req->contenido(), req->token());
        res->set_exito(ok);
        res->set_mensaje(ok ? "Mensaje enviado" : "Error al enviar mensaje");
        return Status::OK;
    }

    Status ConsumirMensajeCola(ServerContext*, const mom::AccionConToken* req, mom::MensajeTexto* res) override {
        auto msg = broker.consumirMensajeDeCola(req->nombre(), req->token());
        if (msg.has_value()) {
            res->set_contenido(msg->getContenido());
            res->set_remitente(msg->getRemitente());
            res->set_canal(msg->getCanal());
            res->set_timestamp("...");
        }
        return Status::OK;
    }

    Status ListarColas(ServerContext*, const mom::Token* req, mom::ListaNombres* res) override {
        auto nombres = broker.listarColas();
        for (const auto& nombre : nombres) {
            res->add_nombres(nombre);
        }
        return Status::OK;
    }

    // ========== Servicios de Tópico ==========
    Status CrearTopico(ServerContext*, const mom::AccionConToken* req, mom::RespuestaSimple* res) override {
        bool ok = broker.crearTopicoDesdeCliente(req->nombre(), req->token());
        res->set_exito(ok);
        res->set_mensaje(ok ? "Tópico creado" : "Error al crear tópico");
        return Status::OK;
    }

    Status EliminarTopico(ServerContext*, const mom::AccionConToken* req, mom::RespuestaSimple* res) override {
        bool ok = broker.eliminarTopicoDesdeCliente(req->nombre(), req->token());
        res->set_exito(ok);
        res->set_mensaje(ok ? "Tópico eliminado" : "Error al eliminar tópico");
        return Status::OK;
    }

    Status SuscribirseTopico(ServerContext*, const mom::AccionConToken* req, mom::RespuestaSimple* res) override {
        bool ok = broker.suscribirATopicoDesdeCliente(req->nombre(), req->token());
        res->set_exito(ok);
        res->set_mensaje(ok ? "Suscripción exitosa" : "Error al suscribirse");
        return Status::OK;
    }

    Status PublicarEnTopico(ServerContext*, const mom::MensajeConToken* req, mom::RespuestaSimple* res) override {
        bool ok = broker.publicarEnTopicoDesdeCliente(req->nombre(), req->contenido(), req->token());
        res->set_exito(ok);
        res->set_mensaje(ok ? "Mensaje publicado" : "Error al publicar mensaje");
        return Status::OK;
    }

    Status ConsumirDesdeTopico(ServerContext*, const mom::AccionConToken* req, mom::ListaMensajes* res) override {
        auto mensajes = broker.consumirDesdeTopico(req->nombre(), req->token());
        for (const auto& m : mensajes) {
            mom::MensajeTexto* nuevo = res->add_mensajes();
            nuevo->set_contenido(m.getContenido());
            nuevo->set_remitente(m.getRemitente());
            nuevo->set_canal(m.getCanal());
            nuevo->set_timestamp("...");
        }
        return Status::OK;
    }

    Status ListarTopicos(ServerContext*, const mom::Token* req, mom::ListaNombres* res) override {
        auto nombres = broker.listarTopicos();
        for (const auto& nombre : nombres) {
            res->add_nombres(nombre);
        }
        return Status::OK;
    }

    // ========== Replicación ==========
    Status EnviarEventoReplica(ServerContext*, const mom::EventoReplica* req, mom::RespuestaSimple* res) override {
        std::cout << "[Replica recibida] ID: " << req->id()
                  << ", tipo: " << req->tipo()
                  << ", entidad: " << req->entidad()
                  << ", origen: " << req->id_origen() << "\n";
    
        if (req->id_origen() == nodoActual) {
            std::cout << "[Replica ignorada] El evento viene del mismo nodo. No procesar.\n";
            res->set_exito(false);
            res->set_mensaje("Evento ignorado: mismo nodo origen");
            return Status::OK;
        }
    
        bool ok = broker.registrarEvento(req->tipo(), req->entidad(), req->datos(), true, req->id_origen());
    
        if (ok) {
            Broker::Evento evento;
            evento.id = req->id();
            evento.tipo = req->tipo();
            evento.entidad = req->entidad();
            evento.datos = req->datos();
            evento.timestamp = req->timestamp();
            evento.id_origen = req->id_origen();
    
            ok = broker.aplicarEvento(evento);
    
            std::cout << (ok ? "[Replica aplicada] Evento ID " + std::to_string(evento.id) + " aplicado correctamente.\n"
                             : "[Replica error] Fallo al aplicar evento ID " + std::to_string(evento.id) + ".\n");
        } else {
            std::cout << "[Replica error] Fallo al registrar evento ID " << req->id() << ".\n";
        }
    
        res->set_exito(ok);
        res->set_mensaje(ok ? "Evento replicado y aplicado" : "Error aplicando evento");
    
        return Status::OK;
    }
    
    

    Status SolicitarEventos(ServerContext*, const mom::EventosRequest* req, mom::ListaEventos* res) override {
        auto eventos = broker.obtenerEventosDesde(req->desde_timestamp());
        for (const auto& e : eventos) {
            mom::EventoReplica* nuevo = res->add_eventos();
            nuevo->set_id(e.id);
            nuevo->set_tipo(e.tipo);
            nuevo->set_entidad(e.entidad);
            nuevo->set_datos(e.datos);
            nuevo->set_timestamp(e.timestamp);
            nuevo->set_id_origen(e.id_origen);
        }
        return Status::OK;
    }

    Status SolicitarPendientes(ServerContext*, const mom::PendientesRequest* req, mom::ListaEventos* res) override {
        auto pendientes = broker.obtenerEventosPendientesParaNodo(req->nodo_destino());
    
        for (const auto& id_evento : pendientes) {
            auto evento_opt = broker.obtenerEventoPorId(id_evento);
            if (evento_opt.has_value()) {
                const auto& evento = evento_opt.value();
                mom::EventoReplica* nuevo = res->add_eventos();
                nuevo->set_id(evento.id);
                nuevo->set_tipo(evento.tipo);
                nuevo->set_entidad(evento.entidad);
                nuevo->set_datos(evento.datos);
                nuevo->set_timestamp(evento.timestamp);
                nuevo->set_id_origen(evento.id_origen);
            }
        }
    
        return Status::OK;
    }
        
};


void iniciarReplicacion(const std::string& nodoActual, NodoManager& manager, Broker& broker) {
    std::thread([nodoActual](NodoManager& managerRef, Broker& brokerRef) {
        while (true) {
            std::cout << "[Replicación] Buscando eventos no replicados...\n";
            auto eventos = brokerRef.obtenerEventosNoReplicados();
            auto otrosNodos = managerRef.obtenerOtrosNodos(nodoActual);

            std::cout << "[Replicación] Encontrados " << eventos.size() << " eventos no replicados.\n";

            for (const auto& evento : eventos) {
                for (const auto& nodo : otrosNodos) {
                    
                    auto pendientesNodo = brokerRef.obtenerEventosPendientesParaNodo(nodo);

                    if (std::find(pendientesNodo.begin(), pendientesNodo.end(), evento.id) != pendientesNodo.end()) {
                        // ➡️ El evento está pendiente para este nodo, ENVIAR
                        auto stub = managerRef.obtenerStub(nodo);
                        if (!stub) {
                            std::cout << "[Replicación] ERROR: No se pudo obtener stub para nodo " << nodo << "\n";
                            continue;
                        }

                        std::cout << "[Replicación] Enviando evento ID " << evento.id << " a nodo " << nodo << "...\n";

                        mom::EventoReplica req;
                        req.set_id(evento.id);
                        req.set_tipo(evento.tipo);
                        req.set_entidad(evento.entidad);
                        req.set_datos(evento.datos);
                        req.set_timestamp(evento.timestamp);
                        req.set_id_origen(nodoActual);

                        mom::RespuestaSimple res;
                        grpc::ClientContext context;
                        grpc::Status status = stub->EnviarEventoReplica(&context, req, &res);

                        if (status.ok() && res.exito()) {
                            std::cout << "[Replicación] Evento ID " << evento.id << " replicado exitosamente en " << nodo << ".\n";

                            brokerRef.marcarPendienteComoReplicado(evento.id, nodo);

                            if (brokerRef.estaEventoCompletamenteReplicado(evento.id)) {
                                brokerRef.marcarEventoComoReplicado(evento.id);
                                std::cout << "[Replicación] Evento ID " << evento.id << " replicado en TODOS los nodos.\n";
                            }
                        } else {
                            std::cout << "[Replicación] ERROR al replicar evento ID " << evento.id << " en " << nodo 
                                      << ". Error: " << status.error_message() << "\n";
                        }
                    } else {
                        // ➡️ El evento ya fue replicado a este nodo
                        std::cout << "[Replicación] Evento ID " << evento.id << " ya replicado a " << nodo << ". Saltando.\n";
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }, std::ref(manager), std::ref(broker)).detach();
}

bool verificarPuerto(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return true;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bool inUse = bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0;
    close(sock);
    return inUse;
}

int buscarPuertoDisponible(int inicio, int fin) {
    for (int port = inicio; port <= fin; ++port) {
        if (!verificarPuerto(port)) {
            return port;
        }
    }
    return -1; // No hay puertos disponibles en el rango
}


int main(int argc, char** argv) {
    // Leer IP y paths desde argumentos o usar por defecto
    std::string ip = (argc > 1) ? argv[1] : "0.0.0.0";  // IP pública o privada del nodo
    std::string data_dir = (argc > 2) ? argv[2] : "./data";  // Carpeta para DB y nodos.txt
    std::string nodos_path = data_dir + "/nodos.txt";

    int port = (argc > 3) ? std::stoi(argv[3]) : 50051;

    if (port == -1) {
        return 1;
    }

    std::filesystem::create_directories(data_dir);
    std::string db_path = data_dir + "/mom" + std::to_string(port) + ".db";
    std::string nodoActual = ip + ":" + std::to_string(port);

    NodoManager nodoManager(nodoActual, nodos_path);
    Broker broker(db_path, nodoActual, &nodoManager);
    iniciarReplicacion(nodoActual, nodoManager, broker);

    std::string server_address = nodoActual;
    MomServiceImpl service(broker, nodoActual);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "✅ Servidor MOM escuchando en " << server_address << std::endl;
    server->Wait();
    return 0;
}


   