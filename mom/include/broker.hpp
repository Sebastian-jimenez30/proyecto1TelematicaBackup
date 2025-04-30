#ifndef BROKER_HPP
#define BROKER_HPP

#include <unordered_map>
#include <optional>
#include <memory>
#include "user.hpp"
#include "queue.hpp"
#include "topic.hpp"
#include "persistence.hpp"
#include "NodoManager.hpp" // 🔵 Agregar para usar NodoManager

class Broker {
public:
    Broker(const std::string& db_path, const std::string& nodoActual, NodoManager* nodoManager);

    // ================== Usuarios ==================
    bool registrarUsuario(const std::string& username, const std::string& password);
    bool autenticarUsuario(const std::string& username, const std::string& password, std::string& token);
    bool guardarTokenReplica(const std::string& username, const std::string& token, const std::string& expiracion);
    bool verificarToken(const std::string& token, std::string& username);

    // ================== Colas ==================

    // Cliente
    bool crearColaDesdeCliente(const std::string& nombre, const std::string& token);
    bool eliminarColaDesdeCliente(const std::string& nombre, const std::string& token);
    bool autorizarColaDesdeCliente(const std::string& nombre, const std::string& usernameObjetivo, const std::string& token);
    bool enviarMensajeAColaDesdeCliente(const std::string& nombre, const std::string& contenido, const std::string& token);
    std::optional<Mensaje> consumirMensajeDeCola(const std::string& nombre, const std::string& token);

    // Replicación
    bool crearColaDesdeReplica(const std::string& nombre, const std::string& username);
    bool eliminarColaDesdeReplica(const std::string& nombre, const std::string& username);
    bool autorizarColaDesdeReplica(const std::string& nombre, const std::string& usernameObjetivo);
    bool enviarMensajeAColaDesdeReplica(const std::string& nombre, const Mensaje& mensaje);

    // ================== Tópicos ==================

    // Cliente
    bool crearTopicoDesdeCliente(const std::string& nombre, const std::string& token);
    bool eliminarTopicoDesdeCliente(const std::string& nombre, const std::string& token);
    bool suscribirATopicoDesdeCliente(const std::string& nombre, const std::string& token);
    bool publicarEnTopicoDesdeCliente(const std::string& nombre, const std::string& contenido, const std::string& token);
    std::vector<Mensaje> consumirDesdeTopico(const std::string& nombre, const std::string& token);

    // Replicación
    bool crearTopicoDesdeReplica(const std::string& nombre, const std::string& username);
    bool eliminarTopicoDesdeReplica(const std::string& nombre, const std::string& username);
    bool suscribirATopicoDesdeReplica(const std::string& nombre, const std::string& username);
    bool publicarEnTopicoDesdeReplica(const std::string& nombre, const Mensaje& mensaje);

    // ================== Listados ==================
    std::vector<std::string> listarTopicos();
    std::vector<std::string> listarColas();

    // ================== Replicación de Eventos ==================
    struct Evento {
        int id;
        std::string tipo;
        std::string entidad;
        std::string datos;
        std::string timestamp;
        std::string id_origen;
    };

    bool aplicarEvento(const Evento& evento);
    std::vector<Evento> obtenerEventosNoReplicados();
    bool marcarEventoComoReplicado(int evento_id);
    bool registrarEvento(const std::string& tipo, const std::string& entidad, const std::string& datos, bool esExterno = false, const std::string& idOrigen = "");
    std::vector<Evento> obtenerEventosDesde(const std::string& desde_timestamp);
    std::vector<Evento> obtenerEventosExternosNoAplicados();
    bool marcarEventoComoReplicadoLocal(int evento_id);
    void aplicarEventosExternosPendientes();

    std::vector<int> obtenerEventosPendientesParaNodo(const std::string& nodo_destino);
    std::optional<Evento> obtenerEventoPorId(int evento_id);
    bool marcarPendienteComoReplicado(int evento_id, const std::string& nodo_destino);
    bool estaEventoCompletamenteReplicado(int evento_id);

private:
    Persistencia persistencia;
    std::unordered_map<std::string, std::shared_ptr<Cola>> colas;
    std::unordered_map<std::string, std::shared_ptr<Topico>> topicos;
    std::string nodoActual;         // 🔵 Identificador IP:Puerto del nodo actual
    NodoManager* nodoManager;       // 🔵 Puntero al manejador de nodos

    bool autenticarYObtenerUsuario(const std::string& token, Usuario& usuario);

    // Métodos internos para manejar correctamente creación en replicación
    bool cargarCola(const std::string& nombre, const std::string& creador);
    bool cargarTopico(const std::string& nombre, const std::string& creador);
};

#endif
