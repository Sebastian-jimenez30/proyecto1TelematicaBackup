#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <jwt-cpp/jwt.h>

class Usuario {
public:
    Usuario(const std::string& username);

    std::string getUsername() const;
    std::string generarToken();
    static bool verificarToken(const std::string& token, std::string& username_out);
    std::chrono::system_clock::time_point getExpiracion() const;

private:
    std::string username;
    std::chrono::system_clock::time_point expiracion;
    inline static const std::string SECRET = "[5>9{3B(Rf1h";
};

#endif 