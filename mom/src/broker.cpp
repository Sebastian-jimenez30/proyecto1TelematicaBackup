#include "broker.hpp"
#include "message.hpp"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "picojson/picojson.h"

Broker::Broker(const std::string& db_path, const std::string& nodoActual_, NodoManager* nodoManager_)
    : persistencia(db_path), nodoActual(nodoActual_), nodoManager(nodoManager_) {
    persistencia.inicializarBaseDeDatos();
}

// ================== Usuarios ==================

bool Broker::registrarUsuario(const std::string& username, const std::string& password) {
    return persistencia.crearUsuario(username, password);
}

bool Broker::autenticarUsuario(const std::string& username, const std::string& password, std::string& token) {
    if (persistencia.verificarCredenciales(username, password)) {
        Usuario user(username);
        token = user.generarToken();

        std::time_t exp_time = std::chrono::system_clock::to_time_t(user.getExpiracion());
        std::stringstream ss;
        ss << std::put_time(std::localtime(&exp_time), "%F %T");
        std::string expiracion = ss.str();

        persistencia.guardarToken(username, token, expiracion);
        return true;
    }
    return false;
}

bool Broker::guardarTokenReplica(const std::string& username, const std::string& token, const std::string& expiracion) {
    return persistencia.guardarToken(username, token, expiracion);
}

bool Broker::verificarToken(const std::string& token, std::string& username) {
    if (!Usuario::verificarToken(token, username)) return false;
    return persistencia.validarTokenActivo(token);
}

bool Broker::autenticarYObtenerUsuario(const std::string& token, Usuario& usuario) {
    std::string username;
    if (!verificarToken(token, username)) return false;
    usuario = Usuario(username);
    return true;
}

// ================== Colas ==================

// Cliente

bool Broker::crearColaDesdeCliente(const std::string& nombre, const std::string& token) {
    Usuario usuario("");
    if (!autenticarYObtenerUsuario(token, usuario)) return false;

    if (colas.find(nombre) != colas.end()) return false;

    auto nueva = std::make_shared<Cola>(nombre, usuario, persistencia);
    colas[nombre] = nueva;
    persistencia.registrarLog(usuario.getUsername(), "Creó cola '" + nombre + "'");

    picojson::object datos;
    datos["username"] = picojson::value(usuario.getUsername());
    datos["nombre"] = picojson::value(nombre);

    registrarEvento("crear_cola", nombre, picojson::value(datos).serialize());

    return true;
}

bool Broker::eliminarColaDesdeCliente(const std::string& nombre, const std::string& token) {
    Usuario usuario("");
    if (!autenticarYObtenerUsuario(token, usuario)) return false;

    auto it = colas.find(nombre);
    if (it != colas.end() && it->second->eliminarDesdeCliente(usuario)) {
        colas.erase(it);

        persistencia.registrarLog(usuario.getUsername(), "Eliminó cola '" + nombre + "'");

        picojson::object datos;
        datos["username"] = picojson::value(usuario.getUsername());
        datos["nombre"] = picojson::value(nombre);
        registrarEvento("eliminar_cola", nombre, picojson::value(datos).serialize());

        return true;
    }

    return false;
}


bool Broker::autorizarColaDesdeCliente(const std::string& nombre, const std::string& usernameObjetivo, const std::string& token) {
    Usuario usuario("");
    if (!autenticarYObtenerUsuario(token, usuario)) return false;

    auto it = colas.find(nombre);
    if (it == colas.end()) return false;

    if (it->second->getCreadorUsername() != usuario.getUsername()) return false;

    bool ok = it->second->autorizarUsuarioDesdeCliente(usernameObjetivo);
    if (ok) {
        picojson::object datos;
        datos["nombre"] = picojson::value(nombre);
        datos["usuarioObjetivo"] = picojson::value(usernameObjetivo);

        registrarEvento("autorizar_cola", nombre, picojson::value(datos).serialize());
    }

    return ok;
}


bool Broker::enviarMensajeAColaDesdeCliente(const std::string& nombre, const std::string& contenido, const std::string& token) {
    Usuario usuario("");
    if (!autenticarYObtenerUsuario(token, usuario)) return false;

    auto it = colas.find(nombre);
    if (it == colas.end()) return false;

    Mensaje msg(usuario.getUsername(), contenido, nombre, "cola", std::chrono::system_clock::now());
    bool ok = it->second->encolarDesdeCliente(msg);

    if (ok) {
        picojson::object datos;
        datos["nombre"] = picojson::value(nombre);
        datos["username"] = picojson::value(usuario.getUsername());
        datos["contenido"] = picojson::value(contenido);

        registrarEvento("enviar_mensaje_cola", nombre, picojson::value(datos).serialize());
    }

    return ok;
}

std::optional<Mensaje> Broker::consumirMensajeDeCola(const std::string& nombre, const std::string& token) {
    Usuario usuario("");
    if (!autenticarYObtenerUsuario(token, usuario)) return std::nullopt;

    auto it = colas.find(nombre);
    if (it != colas.end() && it->second->usuarioPuedeConsumir(usuario) && it->second->hayMensajes()) {
        Mensaje m = it->second->desencolar();
        persistencia.registrarLog(usuario.getUsername(), "Consumió mensaje de '" + nombre + "'");
        return m;
    }
    return std::nullopt;
}

// Replicación

bool Broker::crearColaDesdeReplica(const std::string& nombre, const std::string& username) {
    return cargarCola(nombre, username);
}

bool Broker::eliminarColaDesdeReplica(const std::string& nombre, const std::string& username) {
    auto it = colas.find(nombre);
    if (it != colas.end()) colas.erase(it);
    return persistencia.eliminarCola(nombre, username);
}

bool Broker::autorizarColaDesdeReplica(const std::string& nombre, const std::string& usernameObjetivo) {
    auto it = colas.find(nombre);
    if (it == colas.end()) return false;
    
    return it->second->autorizarUsuarioDesdeReplica(usernameObjetivo);
}

bool Broker::enviarMensajeAColaDesdeReplica(const std::string& nombre, const Mensaje& mensaje) {
    auto it = colas.find(nombre);
    if (it == colas.end()) return false;

    return it->second->encolarDesdeReplica(mensaje);
}

// ================== Tópicos ==================

// Cliente

bool Broker::crearTopicoDesdeCliente(const std::string& nombre, const std::string& token) {
    Usuario usuario("");
    if (!autenticarYObtenerUsuario(token, usuario)) return false;

    if (topicos.find(nombre) != topicos.end()) return false;

    auto nuevo = std::make_shared<Topico>(nombre, usuario, persistencia);
    topicos[nombre] = nuevo;

    picojson::object datos;
    datos["nombre"] = picojson::value(nombre);
    datos["username"] = picojson::value(usuario.getUsername());

    registrarEvento("crear_topico", nombre, picojson::value(datos).serialize());

    return true;
}

bool Broker::eliminarTopicoDesdeCliente(const std::string& nombre, const std::string& token) {
    Usuario usuario("");
    if (!autenticarYObtenerUsuario(token, usuario)) return false;

    auto it = topicos.find(nombre);
    if (it == topicos.end() || !it->second->puedeEliminar(usuario)) return false;

    bool ok = it->second->eliminarDesdeCliente(usuario);
    if (ok) {
        topicos.erase(it);

        picojson::object datos;
        datos["nombre"] = picojson::value(nombre);
        datos["username"] = picojson::value(usuario.getUsername());

        registrarEvento("eliminar_topico", nombre, picojson::value(datos).serialize());
    }

    return ok;
}

bool Broker::suscribirATopicoDesdeCliente(const std::string& nombre, const std::string& token) {
    Usuario usuario("");
    if (!autenticarYObtenerUsuario(token, usuario)) return false;

    auto it = topicos.find(nombre);
    if (it == topicos.end()) return false;

    bool ok = it->second->suscribirDesdeCliente(usuario);
    if (ok) {
        picojson::object datos;
        datos["nombre"] = picojson::value(nombre);
        datos["username"] = picojson::value(usuario.getUsername());

        registrarEvento("suscribir_topico", nombre, picojson::value(datos).serialize());
    }

    return ok;
}

bool Broker::publicarEnTopicoDesdeCliente(const std::string& nombre, const std::string& contenido, const std::string& token) {
    Usuario usuario("");
    if (!autenticarYObtenerUsuario(token, usuario)) return false;

    auto it = topicos.find(nombre);
    if (it == topicos.end()) return false;

    Mensaje msg(usuario.getUsername(), contenido, nombre, "topico", std::chrono::system_clock::now());
    bool ok = it->second->publicarDesdeCliente(msg);  // CAMBIADO A publicarDesdeCliente

    if (ok) {
        picojson::object datos;
        datos["nombre"] = picojson::value(nombre);
        datos["username"] = picojson::value(usuario.getUsername());
        datos["contenido"] = picojson::value(contenido);

        registrarEvento("publicar_topico", nombre, picojson::value(datos).serialize());
    }

    return ok;
}


std::vector<Mensaje> Broker::consumirDesdeTopico(const std::string& nombre, const std::string& token) {
    Usuario usuario("");
    if (!autenticarYObtenerUsuario(token, usuario)) return {};

    auto it = topicos.find(nombre);
    if (it != topicos.end()) {
        auto mensajes = it->second->obtenerMensajesPara(usuario);
        if (!mensajes.empty()) {
            persistencia.registrarLog(usuario.getUsername(), "Consumió mensajes del tópico '" + nombre + "'");
        }
        return mensajes;
    }
    return {};
}

// Replicación

bool Broker::crearTopicoDesdeReplica(const std::string& nombre, const std::string& username) {
    return cargarTopico(nombre, username);
}

bool Broker::eliminarTopicoDesdeReplica(const std::string& nombre, const std::string& username) {
    auto it = topicos.find(nombre);
    if (it != topicos.end()) topicos.erase(it);
    return persistencia.eliminarTopico(nombre, username);
}

bool Broker::suscribirATopicoDesdeReplica(const std::string& nombre, const std::string& username) {
    auto it = topicos.find(nombre);
    if (it == topicos.end()) return false;

    Usuario usuario(username);
    return it->second->suscribirDesdeReplica(usuario);
}

bool Broker::publicarEnTopicoDesdeReplica(const std::string& nombre, const Mensaje& mensaje) {
    auto it = topicos.find(nombre);
    if (it == topicos.end()) return false;

    return it->second->publicarDesdeCliente(mensaje);
}

// ================== Listados ==================

std::vector<std::string> Broker::listarTopicos() {
    return persistencia.obtenerNombresTopicos();
}

std::vector<std::string> Broker::listarColas() {
    return persistencia.obtenerNombresColas();
}

// ================== Replicación de Eventos ==================

bool Broker::aplicarEvento(const Evento& evento) {
    picojson::value v;
    std::string err = picojson::parse(v, evento.datos);
    if (!err.empty() || !v.is<picojson::object>()) return false;
    auto obj = v.get<picojson::object>();

    if (evento.tipo == "crear_cola") {
        return crearColaDesdeReplica(obj["nombre"].get<std::string>(), obj["username"].get<std::string>());
    } else if (evento.tipo == "eliminar_cola") {
        return eliminarColaDesdeReplica(obj["nombre"].get<std::string>(), obj["username"].get<std::string>());
    } else if (evento.tipo == "autorizar_cola") {
        return autorizarColaDesdeReplica(obj["nombre"].get<std::string>(), obj["usuarioObjetivo"].get<std::string>());
    } else if (evento.tipo == "enviar_mensaje_cola") {
        Mensaje msg(
            obj["username"].get<std::string>(),
            obj["contenido"].get<std::string>(),
            obj["nombre"].get<std::string>(),
            "cola",
            std::chrono::system_clock::now()
        );
        return enviarMensajeAColaDesdeReplica(obj["nombre"].get<std::string>(), msg);
    } else if (evento.tipo == "crear_topico") {
        return crearTopicoDesdeReplica(obj["nombre"].get<std::string>(), obj["username"].get<std::string>());
    } else if (evento.tipo == "eliminar_topico") {
        return eliminarTopicoDesdeReplica(obj["nombre"].get<std::string>(), obj["username"].get<std::string>());
    } else if (evento.tipo == "suscribir_topico") {
        return suscribirATopicoDesdeReplica(obj["nombre"].get<std::string>(), obj["username"].get<std::string>());
    } else if (evento.tipo == "publicar_topico") {
        Mensaje msg(
            obj["username"].get<std::string>(),
            obj["contenido"].get<std::string>(),
            obj["nombre"].get<std::string>(),
            "topico",
            std::chrono::system_clock::now()
        );
        return publicarEnTopicoDesdeReplica(obj["nombre"].get<std::string>(), msg);
    }

    return false;
}

bool Broker::registrarEvento(const std::string& tipo, const std::string& entidad, const std::string& datos, bool esExterno, const std::string& idOrigen) {
    // 1. Registrar el evento normal
    bool exito = persistencia.registrarEvento(tipo, entidad, datos, esExterno, idOrigen);
    if (!exito) return false;

    // 2. Si el evento es local (esExterno == false), registramos pendientes
    if (!esExterno) {
        // Obtener ID del último evento insertado
        int ultimoEventoId = persistencia.obtenerUltimoEventoId();

        // 🔵 Obtener todos los otros nodos desde el NodoManager
        auto otrosNodos = nodoManager->obtenerOtrosNodos(nodoActual);

        for (const auto& nodo : otrosNodos) {
            persistencia.registrarPendiente(ultimoEventoId, nodo);
        }
    }

    return true;
}

std::vector<Broker::Evento> Broker::obtenerEventosNoReplicados() {
    std::vector<Evento> eventos;
    std::string sql = R"SQL(
        SELECT id, tipo, entidad, datos, timestamp, id_origen
        FROM eventos_replicacion
        WHERE replicado = 0
        ORDER BY timestamp ASC;
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(persistencia.getDB(), sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Evento e;
            e.id = sqlite3_column_int(stmt, 0);
            e.tipo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            e.entidad = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            e.datos = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            e.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            e.id_origen = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            eventos.push_back(e);
        }
        sqlite3_finalize(stmt);
    }

    return eventos;
}

bool Broker::marcarEventoComoReplicado(int evento_id) {
    return persistencia.forzarMarcarEventoComoReplicado(evento_id);
}

std::vector<Broker::Evento> Broker::obtenerEventosDesde(const std::string& desde_timestamp) {
    std::vector<Evento> eventos;
    std::string sql = R"SQL(
        SELECT id, tipo, entidad, datos, timestamp, id_origen
        FROM eventos_replicacion
        WHERE timestamp > ?
        ORDER BY timestamp ASC;
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(persistencia.getDB(), sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, desde_timestamp.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Evento e;
            e.id = sqlite3_column_int(stmt, 0);
            e.tipo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            e.entidad = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            e.datos = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            e.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            e.id_origen = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            eventos.push_back(e);
        }
        sqlite3_finalize(stmt);
    }

    return eventos;
}

std::vector<Broker::Evento> Broker::obtenerEventosExternosNoAplicados() {
    std::vector<Evento> eventos;

    auto ids = persistencia.obtenerEventosPendientesParaNodo(nodoActual); 

    for (int id : ids) {
        std::string sql = R"SQL(
            SELECT id, tipo, entidad, datos, timestamp, id_origen
            FROM eventos_replicacion
            WHERE id = ?
        )SQL";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(persistencia.getDB(), sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                Evento e;
                e.id = sqlite3_column_int(stmt, 0);
                e.tipo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                e.entidad = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                e.datos = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                e.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                e.id_origen = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                eventos.push_back(e);
            }
            sqlite3_finalize(stmt);
        }
    }

    return eventos;
}

bool Broker::marcarEventoComoReplicadoLocal(int evento_id) {
    return persistencia.marcarPendienteComoReplicado(evento_id, nodoActual);
}

void Broker::aplicarEventosExternosPendientes() {
    auto eventos = obtenerEventosExternosNoAplicados();
    for (const auto& evento : eventos) {
        if (aplicarEvento(evento)) {
            marcarEventoComoReplicadoLocal(evento.id);
        }
    }
}

// ================== Funciones internas ==================

bool Broker::cargarCola(const std::string& nombre, const std::string& creador) {
    if (colas.find(nombre) != colas.end()) return false;
    Usuario usuario(creador);
    auto nueva = std::make_shared<Cola>(nombre, usuario, persistencia);
    colas[nombre] = nueva;
    return true;
}

bool Broker::cargarTopico(const std::string& nombre, const std::string& creador) {
    if (topicos.find(nombre) != topicos.end()) return false;
    Usuario usuario(creador);
    auto nuevo = std::make_shared<Topico>(nombre, usuario, persistencia);
    topicos[nombre] = nuevo;
    return true;
}

std::vector<int> Broker::obtenerEventosPendientesParaNodo(const std::string& nodo_destino) {
    return persistencia.obtenerEventosPendientesParaNodo(nodo_destino);
}

std::optional<Broker::Evento> Broker::obtenerEventoPorId(int evento_id) {
    std::string sql = R"SQL(
        SELECT id, tipo, entidad, datos, timestamp, id_origen
        FROM eventos_replicacion
        WHERE id = ?
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(persistencia.getDB(), sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, evento_id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            Evento e;
            e.id = sqlite3_column_int(stmt, 0);
            e.tipo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            e.entidad = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            e.datos = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            e.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            e.id_origen = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            sqlite3_finalize(stmt);
            return e;
        }
        sqlite3_finalize(stmt);
    }
    return std::nullopt;
}

bool Broker::marcarPendienteComoReplicado(int evento_id, const std::string& nodo_destino) {
    return persistencia.marcarPendienteComoReplicado(evento_id, nodo_destino);
}

bool Broker::estaEventoCompletamenteReplicado(int evento_id) {
    return persistencia.estaEventoCompletamenteReplicado(evento_id);
}
