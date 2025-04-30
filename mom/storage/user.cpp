#include "user.hpp"
#include <iostream>
#include <chrono>

Usuario::Usuario(const std::string& username) : username(username) {}

std::string Usuario::getUsername() const {
    return username;
}

std::string Usuario::generarToken() {
    using namespace std::chrono;

    auto now = system_clock::now();
    expiracion = now + minutes(60); 

    return jwt::create()
        .set_issuer("mom")
        .set_subject(username)
        .set_issued_at(now)
        .set_expires_at(expiracion)
        .sign(jwt::algorithm::hs256{SECRET});
}

bool Usuario::verificarToken(const std::string& token, std::string& username_out){
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{SECRET})
            .with_issuer("mom");

        verifier.verify(decoded);

        username_out = decoded.get_subject();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::chrono::system_clock::time_point Usuario::getExpiracion() const {
    return expiracion;
}