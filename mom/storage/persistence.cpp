#include "persistence.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>     
#include <ctime>       
#include <chrono>

Persistencia::Persistencia(const std::string& db_path) {

    if (sqlite3_open(db_path.c_str(), &db) == SQLITE_OK) {
        conectado == true;
    } else {
        std::cerr << "No se pudo abrir la base de datos: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

Persistencia::~Persistencia() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Persistencia::ejecutar(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Error al ejecutar SQL: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Persistencia::inicializarBaseDeDatos() {
    const std::string sql_usuarios = R"SQL(
        CREATE TABLE IF NOT EXISTS usuarios (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE,
            password_hash TEXT NOT NULL,
            token TEXT,
            token_expiracion TEXT,
            creado_en TEXT
        );
    )SQL";

    const std::string sql_colas = R"SQL(
        CREATE TABLE IF NOT EXISTS colas (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nombre TEXT UNIQUE,
            creador_id INTEGER,
            creado_en TEXT,
            FOREIGN KEY(creador_id) REFERENCES usuarios(id)
        );
    )SQL";

    const std::string sql_topicos = R"SQL(
        CREATE TABLE IF NOT EXISTS topicos (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nombre TEXT UNIQUE,
            creador_id INTEGER,
            creado_en TEXT,
            FOREIGN KEY(creador_id) REFERENCES usuarios(id)
        );
    )SQL";

    const std::string sql_mensajes = R"SQL(
        CREATE TABLE IF NOT EXISTS mensajes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            remitente_id INTEGER,
            canal TEXT,
            tipo TEXT,
            contenido TEXT,
            estado TEXT,
            timestamp TEXT,
            FOREIGN KEY(remitente_id) REFERENCES usuarios(id)
        );
    )SQL";

    const std::string sql_offsets = R"SQL(
        CREATE TABLE IF NOT EXISTS offsets (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            consumidor_id INTEGER,
            topico_id INTEGER,
            offset INTEGER,
            timestamp TEXT,
            FOREIGN KEY(consumidor_id) REFERENCES usuarios(id),
            FOREIGN KEY(topico_id) REFERENCES topicos(id)
        );
    )SQL";

    const std::string sql_autorizaciones = R"SQL(
        CREATE TABLE IF NOT EXISTS autorizaciones (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            usuario_id INTEGER,
            cola_id INTEGER,
            nivel TEXT,
            FOREIGN KEY(usuario_id) REFERENCES usuarios(id),
            FOREIGN KEY(cola_id) REFERENCES colas(id)
        );
    )SQL";

    const std::string sql_suscripciones = R"SQL(
        CREATE TABLE IF NOT EXISTS suscripciones (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            usuario_id INTEGER,
            topico_id INTEGER,
            creado_en TEXT,
            FOREIGN KEY(usuario_id) REFERENCES usuarios(id),
            FOREIGN KEY(topico_id) REFERENCES topicos(id)
        );
    )SQL";

    const std::string sql_logs = R"SQL(
        CREATE TABLE IF NOT EXISTS logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            usuario_id INTEGER,
            actividad TEXT,
            fecha TEXT,
            FOREIGN KEY(usuario_id) REFERENCES usuarios(id)
        );
    )SQL";

    const std::string sql_eventos_replicacion = R"SQL(
        CREATE TABLE IF NOT EXISTS eventos_replicacion (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            tipo TEXT,
            entidad TEXT,
            datos TEXT,
            timestamp TEXT,
            replicado INTEGER DEFAULT 0,
            es_externo INTEGER DEFAULT 0,
            replicado_local INTEGER DEFAULT 0,
            id_origen TEXT
        );
    )SQL";

    const std::string sql_eventos_nodos_pendientes = R"SQL(
        CREATE TABLE IF NOT EXISTS eventos_nodos_pendientes (
            evento_id INTEGER,
            nodo_destino TEXT,
            replicado INTEGER DEFAULT 0,
            PRIMARY KEY (evento_id, nodo_destino),
            FOREIGN KEY(evento_id) REFERENCES eventos_replicacion(id)
        );
    )SQL";

    return ejecutar(sql_usuarios)
        && ejecutar(sql_colas)
        && ejecutar(sql_topicos)
        && ejecutar(sql_mensajes)
        && ejecutar(sql_offsets)
        && ejecutar(sql_autorizaciones)
        && ejecutar(sql_suscripciones)
        && ejecutar(sql_logs)
        && ejecutar(sql_eventos_replicacion)
        && ejecutar(sql_eventos_nodos_pendientes);
}

bool Persistencia::crearUsuario(const std::string& username, const std::string& passwordHash) {
    std::string sql = "INSERT INTO usuarios (username, password_hash, creado_en) VALUES (?, ?, datetime('now'));";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_STATIC);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        return rc == SQLITE_DONE;
    }

    return false;
}

bool Persistencia::verificarCredenciales(const std::string& username, const std::string& passwordHash) {
    std::string sql = "SELECT COUNT(*) FROM usuarios WHERE username = ? AND password_hash = ?;";
    sqlite3_stmt* stmt;
    bool valido = false;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            valido = (count > 0);
        }
        sqlite3_finalize(stmt);
    }

    return valido;
}

bool Persistencia::guardarToken(const std::string& username, const std::string& token, const std::string& expiracion) {
    std::string sql = "UPDATE usuarios SET token = ?, token_expiracion = ? WHERE username = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, expiracion.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, username.c_str(), -1, SQLITE_STATIC);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    return false;
}

bool Persistencia::validarTokenActivo(const std::string& token) {
    std::string sql = "SELECT token_expiracion FROM usuarios WHERE token = ?;";
    sqlite3_stmt* stmt;
    std::string exp_str;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exp_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    if (exp_str.empty()) return false;

    std::tm tm = {};
    std::istringstream ss(exp_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    auto exp_tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    return std::chrono::system_clock::now() < exp_tp;
}

bool Persistencia::crearCola(const std::string& nombreCola, const std::string& creadorUsername) {

    std::string check_sql = "SELECT COUNT(*) FROM colas WHERE nombre = ?;";
    sqlite3_stmt* check_stmt;

    if (sqlite3_prepare_v2(db, check_sql.c_str(), -1, &check_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(check_stmt, 1, nombreCola.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(check_stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(check_stmt, 0);
            sqlite3_finalize(check_stmt);
            if (count > 0) {
                std::cerr << "La cola '" << nombreCola << "' ya existe.\n";
                return false;
            }
        } else {
            sqlite3_finalize(check_stmt);
            return false;
        }
    } else {
        return false;
    }

    std::string insert_sql = R"SQL(
        INSERT INTO colas (nombre, creador_id, creado_en)
        VALUES (?, (SELECT id FROM usuarios WHERE username = ?), datetime('now'));
    )SQL";

    sqlite3_stmt* insert_stmt;
    if (sqlite3_prepare_v2(db, insert_sql.c_str(), -1, &insert_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(insert_stmt, 1, nombreCola.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 2, creadorUsername.c_str(), -1, SQLITE_STATIC);

        int rc = sqlite3_step(insert_stmt);
        sqlite3_finalize(insert_stmt);
        return rc == SQLITE_DONE;
    }

    return false;
}

bool Persistencia::eliminarCola(const std::string& nombreCola, const std::string& username) {
    std::string sql = R"SQL(
        DELETE FROM colas
        WHERE nombre = ? AND creador_id = (
            SELECT id FROM usuarios WHERE username = ?
        );
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, nombreCola.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    return false;
}

bool Persistencia::verificarAutorizacion(const std::string& username, const std::string& colaNombre) {
    std::string sql = R"SQL(
        SELECT COUNT(*) FROM autorizaciones
        WHERE usuario_id = (SELECT id FROM usuarios WHERE username = ?)
          AND cola_id = (SELECT id FROM colas WHERE nombre = ?);
    )SQL";

    sqlite3_stmt* stmt;
    bool autorizado = false;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, colaNombre.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            autorizado = sqlite3_column_int(stmt, 0) > 0;
        }

        sqlite3_finalize(stmt);
    }

    return autorizado;
}

bool Persistencia::autorizarUsuarioParaConsumir(const std::string& username, const std::string& colaNombre) {
    std::string sql = R"SQL(
        INSERT INTO autorizaciones (usuario_id, cola_id, nivel)
        VALUES (
            (SELECT id FROM usuarios WHERE username = ?),
            (SELECT id FROM colas WHERE nombre = ?),
            'lector'
        );
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, colaNombre.c_str(), -1, SQLITE_STATIC);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    return false;
}

bool Persistencia::guardarMensaje(const Mensaje& mensaje) {
    std::string sql = R"SQL(
        INSERT INTO mensajes (remitente_id, canal, tipo, contenido, estado, timestamp)
        VALUES (
            (SELECT id FROM usuarios WHERE username = ?),
            ?, ?, ?, 'enviado', ?
        );
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        // Convertir timestamp
        std::time_t time = std::chrono::system_clock::to_time_t(mensaje.getTimestamp());
        std::stringstream ts;
        ts << std::put_time(std::localtime(&time), "%F %T");

        sqlite3_bind_text(stmt, 1, mensaje.getRemitente().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, mensaje.getCanal().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, mensaje.getTipo().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, mensaje.getContenido().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, ts.str().c_str(), -1, SQLITE_TRANSIENT);           

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    return false;
}

std::vector<Mensaje> Persistencia::cargarMensajesPorCanal(const std::string& canal, const std::string& tipo) {
    std::vector<Mensaje> mensajes;

    std::string sql = R"SQL(
        SELECT id, remitente_id, contenido, canal, tipo, timestamp
        FROM mensajes
        WHERE canal = ? AND tipo = ?
        ORDER BY timestamp ASC;
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, canal.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, tipo.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            std::string remitente = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string contenido = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::string canal = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            std::string tipo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            std::string ts_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

            std::tm tm = {};
            std::istringstream ss(ts_str);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            auto timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

            mensajes.emplace_back(id, remitente, contenido, canal, tipo, timestamp);
        }

        sqlite3_finalize(stmt);
    }

    return mensajes;
}

std::vector<Mensaje> Persistencia::cargarMensajesDesdeOffset(const std::string& topico, int offset, int limite) {
    std::vector<Mensaje> mensajes;

    std::string sql = R"SQL(
        SELECT id, remitente_id, contenido, canal, tipo, timestamp
        FROM mensajes
        WHERE canal = ? AND tipo = 'topico'
        ORDER BY timestamp ASC
        LIMIT ? OFFSET ?;
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, topico.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, limite);
        sqlite3_bind_int(stmt, 3, offset);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            std::string remitente = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string contenido = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::string canal = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            std::string tipo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            std::string ts_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

            std::tm tm = {};
            std::istringstream ss(ts_str);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            auto timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

            mensajes.emplace_back(id, remitente, contenido, canal, tipo, timestamp);
        }

        sqlite3_finalize(stmt);
    }

    return mensajes;
}

bool Persistencia::crearTopico(const std::string& nombreTopico, const std::string& creadorUsername) {
    std::string sql = R"SQL(
        INSERT INTO topicos (nombre, creador_id, creado_en)
        VALUES (?, (SELECT id FROM usuarios WHERE username = ?), datetime('now'));
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, nombreTopico.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, creadorUsername.c_str(), -1, SQLITE_STATIC);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    return false;
}

bool Persistencia::suscribirUsuarioATopico(const std::string& username, const std::string& topicoNombre) {
    std::string sql = R"SQL(
        INSERT OR IGNORE INTO suscripciones (usuario_id, topico_id, creado_en)
        VALUES (
            (SELECT id FROM usuarios WHERE username = ?),
            (SELECT id FROM topicos WHERE nombre = ?),
            datetime('now')
        );
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, topicoNombre.c_str(), -1, SQLITE_STATIC);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    return false;
}

int Persistencia::obtenerOffset(const std::string& username, const std::string& topicoNombre) {
    std::string sql = R"SQL(
        SELECT offset FROM offsets
        WHERE consumidor_id = (SELECT id FROM usuarios WHERE username = ?)
          AND topico_id = (SELECT id FROM topicos WHERE nombre = ?);
    )SQL";

    sqlite3_stmt* stmt;
    int offset = 0;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, topicoNombre.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            offset = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
    }

    return offset;
}

bool Persistencia::actualizarOffset(const std::string& username, const std::string& topicoNombre, int nuevoOffset) {
    std::string sql = R"SQL(
        INSERT INTO offsets (consumidor_id, topico_id, offset, timestamp)
        VALUES (
            (SELECT id FROM usuarios WHERE username = ?),
            (SELECT id FROM topicos WHERE nombre = ?),
            ?, datetime('now')
        )
        ON CONFLICT(consumidor_id, topico_id)
        DO UPDATE SET offset = excluded.offset, timestamp = excluded.timestamp;
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, topicoNombre.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, nuevoOffset);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    return false;
}

bool Persistencia::eliminarTopico(const std::string& nombreTopico, const std::string& username) {
    std::string sql = R"SQL(
        DELETE FROM topicos
        WHERE nombre = ? AND creador_id = (
            SELECT id FROM usuarios WHERE username = ?
        );
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, nombreTopico.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    return false;
}

bool Persistencia::registrarLog(const std::string& username, const std::string& actividad) {
    std::string sql = R"SQL(
        INSERT INTO logs (usuario_id, actividad, fecha)
        VALUES (
            (SELECT id FROM usuarios WHERE username = ?),
            ?,
            datetime('now')
        );
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, actividad.c_str(), -1, SQLITE_STATIC);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    return false;
}

std::vector<std::string> Persistencia::obtenerNombresTopicos() {
    std::vector<std::string> topicos;
    std::string sql = "SELECT nombre FROM topicos ORDER BY nombre ASC;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            topicos.push_back(nombre);
        }
        sqlite3_finalize(stmt);
    }
    return topicos;
}

std::vector<std::string> Persistencia::obtenerNombresColas() {
    std::vector<std::string> colas;
    std::string sql = "SELECT nombre FROM colas ORDER BY nombre ASC;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            colas.push_back(nombre);
        }
        sqlite3_finalize(stmt);
    }
    return colas;
}

bool Persistencia::registrarEvento(const std::string& tipo, const std::string& entidad, const std::string& datos, bool esExterno, const std::string& id_origen) {
    std::string sql = R"SQL(
        INSERT INTO eventos_replicacion (tipo, entidad, datos, timestamp, replicado, es_externo, replicado_local, id_origen)
        VALUES (?, ?, ?, datetime('now'), 0, ?, 0, ?);
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, tipo.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, entidad.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, datos.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, esExterno ? 1 : 0);
        sqlite3_bind_text(stmt, 5, id_origen.c_str(), -1, SQLITE_STATIC);

        int rc = sqlite3_step(stmt);

        if (rc == SQLITE_DONE) {
            // 🔵 Obtener el ID del evento recién insertado
            int evento_id = static_cast<int>(sqlite3_last_insert_rowid(db));

            // 🔵 Registrar pendientes para todos los demás nodos
            std::vector<std::string> otros_nodos = {/* aquí debes pasar tus otros nodos conocidos */};

            for (const auto& nodo : otros_nodos) {
                if (nodo != id_origen) {  // No te replicas a ti mismo
                    registrarPendiente(evento_id, nodo);
                }
            }

            sqlite3_finalize(stmt);
            return true;
        }

        sqlite3_finalize(stmt);
    }
    return false;
}


bool Persistencia::registrarPendiente(int evento_id, const std::string& nodo_destino) {
    std::string sql = R"SQL(
        INSERT OR IGNORE INTO eventos_nodos_pendientes (evento_id, nodo_destino, replicado)
        VALUES (?, ?, 0);
    )SQL";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, evento_id);
        sqlite3_bind_text(stmt, 2, nodo_destino.c_str(), -1, SQLITE_STATIC);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }
    return false;
}

bool Persistencia::marcarPendienteComoReplicado(int evento_id, const std::string& nodo_destino) {
    std::string sql = R"SQL(
        UPDATE eventos_nodos_pendientes
        SET replicado = 1
        WHERE evento_id = ? AND nodo_destino = ?;
    )SQL";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, evento_id);
        sqlite3_bind_text(stmt, 2, nodo_destino.c_str(), -1, SQLITE_STATIC);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }
    return false;
}

std::vector<int> Persistencia::obtenerEventosPendientesParaNodo(const std::string& nodo_destino) {
    std::vector<int> eventos;
    std::string sql = R"SQL(
        SELECT evento_id
        FROM eventos_nodos_pendientes
        WHERE nodo_destino = ? AND replicado = 0;
    )SQL";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, nodo_destino.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int evento_id = sqlite3_column_int(stmt, 0);
            eventos.push_back(evento_id);
        }
        sqlite3_finalize(stmt);
    }
    return eventos;
}

bool Persistencia::estaEventoCompletamenteReplicado(int evento_id) {
    std::string sql = R"SQL(
        SELECT COUNT(*) FROM eventos_nodos_pendientes
        WHERE evento_id = ? AND replicado = 0;
    )SQL";

    sqlite3_stmt* stmt;
    bool completamente = false;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, evento_id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            completamente = (sqlite3_column_int(stmt, 0) == 0);
        }
        sqlite3_finalize(stmt);
    }
    return completamente;
}

bool Persistencia::forzarMarcarEventoComoReplicado(int evento_id) {
    std::string sql = "UPDATE eventos_replicacion SET replicado = 1 WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, evento_id);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }
    return false;
}

int Persistencia::obtenerUltimoEventoId() {
    std::string sql = R"SQL(
        SELECT MAX(id) FROM eventos_replicacion;
    )SQL";

    sqlite3_stmt* stmt;
    int id = -1;  // 🔵 Si no hay eventos aún, devolvemos -1

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    return id;
}

sqlite3* Persistencia::getDB() const {
    return db;
}