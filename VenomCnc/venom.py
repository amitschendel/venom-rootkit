import socket
from dataclasses import dataclass


@dataclass
class Venom:
    venom_socket: socket.socket
    venom_address: tuple
    venom_id: str

    def __str__(self):
        return f"{self.venom_address} - {self.venom_id}"
