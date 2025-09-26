import spidev
import time
import struct

from flat_lawn_mower_status import FlatLawnMowerStatus, FLAT_MOWER_STATUS_FORMAT
from spi_slave import SPISlave

# --- Configuration (must match STM32 settings) ---
SPI_BUS = 0
SPI_DEVICE = 0
SPI_SPEED_HZ = 36000000  # Adjusted to match STM32's 36 MHz (PCLK2/2)
# SPI_MODE = 0b00  # CPOL=0, CPHA=0 (mode 0) - This is implied by default for spidev, but can be set explicitly if needed.

def display_menu():
    print("\n--- Raspberry Pi Debug Menu ---")
    print("1. Receive STM32 status (default)")
    print("2. Send command: Toggle LED (Example)")
    print("3. Send command: Request specific sensor data (Example)")
    print("Q. Quit")
    return input("Enter your choice: ").strip().upper()

def main():
    print("Raspberry Pi SPI Slave Application")
    spi_slave = None
    try:
        spi_slave = SPISlave(bus=SPI_BUS, device=SPI_DEVICE, max_speed_hz=SPI_SPEED_HZ)
        print(f"SPI connection opened on bus {spi_slave.spi.bus}, device {spi_slave.spi.chip_select_pin}.")

        expected_data_size = struct.calcsize(FLAT_MOWER_STATUS_FORMAT)
        print(f"Expecting to receive {expected_data_size} bytes per message.")

        while True:
            choice = display_menu()

            if choice == 'Q':
                break
            elif choice == '2':
                print("Sending Toggle LED command...")
                # Example command: a single byte for command ID
                response = spi_slave.send_command(b'\x01', rx_len=1) # Send 0x01, expect 1 byte response
                print(f"Command sent, STM32 response: {response.hex()}")
            elif choice == '3':
                print("Sending Request Sensor Data command...")
                # Example command: command ID + sensor ID
                response = spi_slave.send_command(b'\x02\x01', rx_len=5) # Request sensor 0x01, expect 5 bytes
                print(f"Command sent, STM32 response: {response.hex()}")
            elif choice == '1':
                print("Waiting for data from STM32...")
                received_raw_data = spi_slave.receive_data(expected_data_size)

                if received_raw_data:
                    try:
                        lawn_mower_status = FlatLawnMowerStatus.from_bytes(received_raw_data)
                        print(f"Received data: {lawn_mower_status}")
                    except struct.error as e:
                        print(f"Error unpacking received data: {e}")
                        print(f"Raw data: {received_raw_data.hex()}")
                else:
                    print("No data received.")
            else:
                print("Invalid choice. Please try again.")

            time.sleep(0.1)

    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        if spi_slave:
            spi_slave.close()
            print("SPI connection closed.")

if __name__ == "__main__":
    main()
