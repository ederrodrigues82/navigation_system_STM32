"""UART client for communication with STM32 over USART2 (RS232 without flow control)."""

import serial


class UARTClient:
    """Client for UART communication with STM32."""

    def __init__(self, port: str = "/dev/ttyAMA0", baudrate: int = 115200, timeout: float = 1.0):
        """
        Initialize UART client.

        Args:
            port: Serial port path. Use /dev/ttyAMA0 for Pi 3/4/5, /dev/ttyS0 for Pi Zero W.
            baudrate: Baud rate (must match STM32, default 115200).
            timeout: Read timeout in seconds.
        """
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.serial = serial.Serial(
            port=port,
            baudrate=baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=timeout,
        )

    def receive_data(self, num_bytes: int) -> bytes:
        """
        Read up to num_bytes from the serial port.

        Args:
            num_bytes: Maximum number of bytes to read.

        Returns:
            Received bytes (may be fewer than num_bytes if timeout occurs).
        """
        return self.serial.read(num_bytes)

    def send_data(self, data: bytes) -> int:
        """
        Send data over the serial port.

        Args:
            data: Bytes to send.

        Returns:
            Number of bytes written.
        """
        return self.serial.write(data)

    def close(self):
        """Close the serial connection."""
        self.serial.close()
