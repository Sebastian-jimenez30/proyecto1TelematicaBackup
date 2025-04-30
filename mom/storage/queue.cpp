#include "queue.hpp"
#include <iostream>

Cola::Cola(const std::string& nombre, Usuario& creador, Persistencia& persistencia, bool registrarEvento)
    : nombre(nombre), creadorUsername(creador.getUsername()), db(persistencia) {

    if (registrarEvento) {
        db.crearCola(nombre, creadorUsername);
        db.autorizarUsuarioParaConsumir(creadorUsername, nombre);
    }
    cargarMensajesDesdeBD();
}

Cola::Cola(const std::string& nombre, const std::string& creadorUsername, Persistencia& persistencia)
    : nombre(nombre), creadorUsername(creadorUsername), db(persistencia) {
    cargarMensajesDesdeBD();
}

const std::string& Cola::getNombre() const {
    return nombre;
}

const std::string& Cola::getCreadorUsername() const {
    return creadorUsername;
}

bool Cola::encolarDesdeCliente(const Mensaje& mensaje) {
    mensajes.push(mensaje);
    return db.guardarMensaje(mensaje);
}

bool Cola::encolarDesdeReplica(const Mensaje& mensaje) {
    mensajes.push(mensaje);
    return db.guardarMensaje(mensaje);  // Si quieres diferenciar puedes crear un guardarMensajeReplica, pero normalmente se usa igual
}

bool Cola::hayMensajes() const {
    return !mensajes.empty();
}

Mensaje Cola::desencolar() {
    if (mensajes.empty()) {
        throw std::runtime_error("No hay mensajes en la cola.");
    }
    Mensaje m = mensajes.front();
    mensajes.pop();
    return m;
}

bool Cola::eliminarDesdeCliente(const Usuario& usuario) {
    if (usuario.getUsername() == creadorUsername) {
        return db.eliminarCola(nombre, usuario.getUsername());
    }
    return false;
}

bool Cola::eliminarDesdeReplica() {
    return db.eliminarCola(nombre, creadorUsername);
}

bool Cola::autorizarUsuarioDesdeCliente(const std::string& usernameAutorizado) {
    return db.autorizarUsuarioParaConsumir(usernameAutorizado, nombre);
}

bool Cola::autorizarUsuarioDesdeReplica(const std::string& usernameAutorizado) {
    return db.autorizarUsuarioParaConsumir(usernameAutorizado, nombre);
}

bool Cola::usuarioPuedeConsumir(const Usuario& usuario) const {
    return db.verificarAutorizacion(usuario.getUsername(), nombre);
}

void Cola::cargarMensajesDesdeBD() {
    std::vector<Mensaje> historico = db.cargarMensajesPorCanal(nombre, "cola");
    for (const auto& msg : historico) {
        mensajes.push(msg);
    }
}
