from flask import Flask, render_template, request, redirect, url_for, session, jsonify
import requests

app = Flask(__name__)
app.secret_key = 'clave-secreta-super-segura'
BASE_URL = "http://44.202.0.227:8080"

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/register', methods=['POST'])
def register():
    data = request.get_json()
    username = data.get('username')
    password = data.get('password')
    res = requests.post(f"{BASE_URL}/register", json={"username": username, "password": password})
    return jsonify(res.json())

@app.route('/login', methods=['POST'])
def login():
    data = request.get_json()
    username = data.get('username')
    password = data.get('password')
    res = requests.post(f"{BASE_URL}/login", json={"username": username, "password": password})
    if res.status_code == 200 and res.json().get("token"):
        session['token'] = res.json()['token']
        session['username'] = username
        return redirect(url_for('panel'))
    return jsonify(res.json())

@app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('index'))

@app.route('/panel')
def panel():
    if 'token' not in session:
        return redirect(url_for('index'))
    return render_template('panel.html', username=session['username'])

@app.route('/accion/<tipo>', methods=['POST'])
def accion(tipo):
    token = session.get('token')
    if not token:
        return jsonify({"error": "No hay token de sesión activo"}), 401

    headers = {"Authorization": f"Bearer {token}"}
    nombre = request.form.get('nombre')
    contenido = request.form.get('contenido')
    usuario = request.form.get('usuario')

    try:
        if tipo == "crear_cola":
            res = requests.post(f"{BASE_URL}/colas", json={"nombre": nombre}, headers=headers)
        elif tipo == "eliminar_cola":
            res = requests.delete(f"{BASE_URL}/colas/{nombre}", headers=headers)
        elif tipo == "crear_topico":
            res = requests.post(f"{BASE_URL}/topicos", json={"nombre": nombre}, headers=headers)
        elif tipo == "eliminar_topico":
            res = requests.delete(f"{BASE_URL}/topicos/{nombre}", headers=headers)
        elif tipo == "listar_colas":
            res = requests.get(f"{BASE_URL}/colas", headers=headers)
        elif tipo == "listar_topicos":
            res = requests.get(f"{BASE_URL}/topicos", headers=headers)
        elif tipo == "autorizar_cola":
            res = requests.post(f"{BASE_URL}/colas/{nombre}/autorizar", json={"usuario": usuario}, headers=headers)
        elif tipo == "enviar_mensaje_cola":
            res = requests.post(f"{BASE_URL}/colas/{nombre}/enviar", json={"contenido": contenido}, headers=headers)
        elif tipo == "consumir_cola":
            res = requests.get(f"{BASE_URL}/colas/{nombre}/consumir", headers=headers)
        elif tipo == "suscribir_topico":
            res = requests.post(f"{BASE_URL}/topicos/{nombre}/suscribir", headers=headers)
        elif tipo == "publicar_topico":
            res = requests.post(f"{BASE_URL}/topicos/{nombre}/publicar", json={"contenido": contenido}, headers=headers)
        elif tipo == "consumir_topico":
            res = requests.get(f"{BASE_URL}/topicos/{nombre}/consumir", headers=headers)
        else:
            return jsonify({"error": "Acción no reconocida"}), 400

        return jsonify(res.json())
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000, debug=True)