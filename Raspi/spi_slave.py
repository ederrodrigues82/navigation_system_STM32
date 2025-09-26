import spidev
import struct

class SPISlave:
    def __init__(self, bus=0, device=0, max_speed_hz=None):
        self.spi = spidev.SpiDev()
        self.spi.open(bus, device)
        if max_speed_hz:
            self.spi.max_speed_hz = max_speed_hz
        self.spi.mode = 0b00  # SPI mode 0 (CPOL=0, CPHA=0) - adjust if STM32 is different

    def receive_data(self, num_bytes):
        # In master mode, we'd use xfer2 or xfer. As a 'slave' from the Pi's perspective
        # (meaning the Pi is receiving), the STM32 is the master initiating transfers.
        # The Pi needs to be ready to send data back, even if it's dummy data.
        # So we use xfer2 which sends and receives simultaneously.
        dummy_data = [0x00] * num_bytes
        received_bytes = self.spi.xfer2(dummy_data)
        return bytes(received_bytes)

    def send_command(self, command_bytes: bytes, rx_len: int = 0) -> bytes:
        # Ensure command_bytes is a list of integers for xfer2
        command_list = list(command_bytes)
        if rx_len > len(command_list):
            # Pad with dummy bytes if more receive bytes are expected than command bytes
            command_list.extend([0x00] * (rx_len - len(command_list)))
        elif rx_len < len(command_list):
            # Truncate if fewer receive bytes are expected
            command_list = command_list[:rx_len]
        
        # xfer2 sends and receives simultaneously. The length of the sent list
        # determines the number of bytes transferred.
        received_bytes = self.spi.xfer2(command_list)
        return bytes(received_bytes)

    def close(self):
        self.spi.close()
