import socket
import threading

from venom_handler import VenomHandler


class Server:
    def __init__(self, listen_ip: str, listen_port: int, venom_handler: VenomHandler):
        self.venom_handler = venom_handler
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind((listen_ip, listen_port))
        self.socket.listen()

    def accept_clients(self):
        while True:
            client, address = self.socket.accept()
            threading.Thread(target=self.venom_handler.handle, args=(client, address)).start()

    def close(self):
        self.socket.close()
