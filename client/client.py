#!/usr/bin/env python3

import requests
import json

BASE_URL = "http://localhost:8080"

def menu():
    print("\n=== Menú Principal ===")
    print("1. Registrar usuario")
    print("2. Iniciar sesión")
    print("3. Crear cola")
    print("4. Eliminar cola")
    print("5. Autorizar usuario en cola")
    print("6. Enviar mensaje a cola")
    print("7. Consumir mensaje de cola")
    print("8. Listar colas")
    print("9. Crear tópico")
    print("10. Eliminar tópico")
    print("11. Suscribirse a tópico")
    print("12. Publicar mensaje en tópico")
    print("13. Consumir mensajes de tópico")
    print("14. Listar tópicos")
    print("15. Salir")
    return input("Selecciona una opción: ")

def registrar_usuario():
    username = input("Nombre de usuario: ")
    password = input("Contraseña: ")
    data = {"username": username, "password": password}
    response = requests.post(f"{BASE_URL}/register", json=data)
    print_response(response)

def iniciar_sesion():
    global token
    username = input("Nombre de usuario: ")
    password = input("Contraseña: ")
    data = {"username": username, "password": password}
    response = requests.post(f"{BASE_URL}/login", json=data)
    if response.status_code == 200:
        token = response.json().get("token")
        print("Inicio de sesión exitoso. Token guardado.")
    else:
        print_response(response)

def crear_cola():
    nombre = input("Nombre de la cola: ")
    headers = {"Authorization": f"Bearer {token}"}
    data = {"nombre": nombre}
    response = requests.post(f"{BASE_URL}/colas", json=data, headers=headers)
    print_response(response)

def eliminar_cola():
    nombre = input("Nombre de la cola a eliminar: ")
    headers = {"Authorization": f"Bearer {token}"}
    response = requests.delete(f"{BASE_URL}/colas/{nombre}", headers=headers)
    print_response(response)

def autorizar_usuario():
    nombre = input("Nombre de la cola: ")
    usuario = input("Usuario a autorizar: ")
    headers = {"Authorization": f"Bearer {token}"}
    data = {"usuario": usuario}
    response = requests.post(f"{BASE_URL}/colas/{nombre}/autorizar", json=data, headers=headers)
    print_response(response)

def enviar_mensaje():
    nombre = input("Nombre de la cola: ")
    contenido = input("Contenido del mensaje: ")
    headers = {"Authorization": f"Bearer {token}"}
    data = {"contenido": contenido}
    response = requests.post(f"{BASE_URL}/colas/{nombre}/enviar", json=data, headers=headers)
    print_response(response)

def consumir_mensaje():
    nombre = input("Nombre de la cola: ")
    headers = {"Authorization": f"Bearer {token}"}
    response = requests.get(f"{BASE_URL}/colas/{nombre}/consumir", headers=headers)
    print_response(response)

def listar_colas():
    headers = {"Authorization": f"Bearer {token}"}
    response = requests.get(f"{BASE_URL}/colas", headers=headers)
    print_response(response)

def crear_topico():
    nombre = input("Nombre del tópico: ")
    headers = {"Authorization": f"Bearer {token}"}
    data = {"nombre": nombre}
    response = requests.post(f"{BASE_URL}/topicos", json=data, headers=headers)
    print_response(response)

def eliminar_topico():
    nombre = input("Nombre del tópico a eliminar: ")
    headers = {"Authorization": f"Bearer {token}"}
    response = requests.delete(f"{BASE_URL}/topicos/{nombre}", headers=headers)
    print_response(response)

def suscribirse_topico():
    nombre = input("Nombre del tópico: ")
    headers = {"Authorization": f"Bearer {token}"}
    data = {"nombre": nombre}
    response = requests.post(f"{BASE_URL}/topicos/{nombre}/suscribir", json=data, headers=headers)
    print_response(response)

def publicar_topico():
    nombre = input("Nombre del tópico: ")
    contenido = input("Contenido del mensaje: ")
    headers = {"Authorization": f"Bearer {token}"}
    data = {"contenido": contenido}
    response = requests.post(f"{BASE_URL}/topicos/{nombre}/publicar", json=data, headers=headers)
    print_response(response)

def consumir_topico():
    nombre = input("Nombre del tópico: ")
    headers = {"Authorization": f"Bearer {token}"}
    response = requests.get(f"{BASE_URL}/topicos/{nombre}/consumir", headers=headers)
    print_response(response)

def listar_topicos():
    headers = {"Authorization": f"Bearer {token}"}
    response = requests.get(f"{BASE_URL}/topicos", headers=headers)
    print_response(response)

def print_response(response):
    try:
        print("\nRespuesta del servidor:")
        print(json.dumps(response.json(), indent=4))
    except json.JSONDecodeError:
        print(response.text)

if __name__ == "__main__":
    token = None
    while True:
        opcion = menu()
        if opcion == "1":
            registrar_usuario()
        elif opcion == "2":
            iniciar_sesion()
        elif opcion == "3":
            if token:
                crear_cola()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "4":
            if token:
                eliminar_cola()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "5":
            if token:
                autorizar_usuario()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "6":
            if token:
                enviar_mensaje()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "7":
            if token:
                consumir_mensaje()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "8":
            if token:
                listar_colas()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "9":
            if token:
                crear_topico()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "10":
            if token:
                eliminar_topico()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "11":
            if token:
                suscribirse_topico()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "12":
            if token:
                publicar_topico()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "13":
            if token:
                consumir_topico()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "14":
            if token:
                listar_topicos()
            else:
                print("Por favor, inicia sesión primero.")
        elif opcion == "15":
            print("Saliendo del cliente...")
            break
        else:
            print("Opción no válida. Intenta de nuevo.")