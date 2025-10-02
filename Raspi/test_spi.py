import spidev
import time

bus = 0
device = 0

try:
    spi = spidev.SpiDev()
    spi.open(bus, device)
    spi.max_speed_hz = 36000000
    spi.mode = 0b00 # SPI mode 0

    print(f"SPI connection opened on bus {bus}, device {device}.")
    print("Sending 4 dummy bytes and reading response (expecting PING from STM32)...")

    # Send 4 dummy bytes to read 4 bytes from slave
    # If STM32 is correctly configured, it will send "PING"
    response = spi.xfer2([0x00, 0x00, 0x00, 0x00])

    print(f"Received data: {bytes(response)}")

except FileNotFoundError:
    print(f"Error: SPI device /dev/spidev{bus}.{device} not found. Ensure SPI is enabled in raspi-config.")
except PermissionError:
    print(f"Error: Permission denied to /dev/spidev{bus}.{device}. Ensure the 'pi' user is in the 'spi' group or run with sudo.")
except Exception as e:
    print(f"An unexpected error occurred: {e}")
finally:
    if 'spi' in locals() and spi.fd: # Check if spi object was created and is open
        spi.close()
        print("SPI connection closed.")