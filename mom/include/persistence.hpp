#ifndef PERSISTENCE_HPP
#define PERSISTENCE_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include "message.hpp"

class Persistencia {
public:
    Persistencia(const std::string& db_path);
    ~Persistencia();

    bool inicializarBaseDeDatos();

    // Usuarios
    bool crearUsuario(const std::string& username, const std::string& passwordHash);
    bool verificarCredenciales(const std::string& username, const std::string& passwordHash);
    bool guardarToken(const std::string& username, const std::string& token, const std::string& expiracion);
    bool validarTokenActivo(const std::string& token);

    // Colas
    bool crearCola(const std::string& nombreCola, const std::string& creadorUsername);
    bool eliminarCola(const std::string& nombreCola, const std::string& username);
    bool autorizarUsuarioParaConsumir(const std::string& username, const std::string& colaNombre);
    bool verificarAutorizacion(const std::string& username, const std::string& colaNombre);

    // Mensajes
    bool guardarMensaje(const Mensaje& mensaje);
    std::vector<Mensaje> cargarMensajesPorCanal(const std::string& canal, const std::string& tipo);
    std::vector<Mensaje> cargarMensajesDesdeOffset(const std::string& topico, int offset, int limite = 10);

    // Topicos
    bool crearTopico(const std::string& nombreTopico, const std::string& creadorUsername);
    bool suscribirUsuarioATopico(const std::string& username, const std::string& topicoNombre);
    int obtenerOffset(const std::string& username, const std::string& topicoNombre);
    bool actualizarOffset(const std::string& username, const std::string& topicoNombre, int nuevoOffset);
    bool eliminarTopico(const std::string& nombreTopico, const std::string& username);

    bool registrarLog(const std::string& username, const std::string& actividad);

    std::vector<std::string> obtenerNombresTopicos();
    std::vector<std::string> obtenerNombresColas();

    // ======== Eventos de replicación =========
    bool registrarEvento(const std::string& tipo, const std::string& entidad, const std::string& datos, bool esExterno = false, const std::string& id_origen = "");

    bool registrarPendiente(int evento_id, const std::string& nodo_destino);
    bool marcarPendienteComoReplicado(int evento_id, const std::string& nodo_destino);

    std::vector<int> obtenerEventosPendientesParaNodo(const std::string& nodo_destino);
    bool estaEventoCompletamenteReplicado(int evento_id);
    bool forzarMarcarEventoComoReplicado(int evento_id);
    int obtenerUltimoEventoId();

    sqlite3* getDB() const;

private:
    sqlite3* db;
    bool conectado;
    bool ejecutar(const std::string& sql);
};

#endif
