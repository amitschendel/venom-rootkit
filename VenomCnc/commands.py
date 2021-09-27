import enum
from dataclasses import dataclass
import struct


class CommandType(enum.Enum):
    HideProcess = 1
    HidePort = 2
    ElevateToken = 3
    ExecuteCommand = 4


@dataclass
class Command:
    size: int
    type: CommandType
    data: bytes

    def build_command(self) -> bytes:
        return struct.pack(f'ii{str(self.size)}s', self.size, self.type, self.data)
