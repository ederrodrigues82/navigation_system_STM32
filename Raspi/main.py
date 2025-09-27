# Raspi/main.py
import debugpy
# This tells debugpy to listen for a connection on port 5678
# and wait until a debugger client connects before continuing execution.
# Change "0.0.0.0" to your Pi's specific IP if you only want to allow
# connections from that IP, but "0.0.0.0" is generally fine for dev.
debugpy.listen(("0.0.0.0", 5678))
print("Waiting for debugger client to attach...")
debugpy.wait_for_client()
print("Debugger attached! Continuing execution.")

import time
import struct
import logging
from flat_lawn_mower_status import FlatLawnMowerStatus, FLAT_MOWER_STATUS_FORMAT
from spi_slave import SPISlave

# Configure logging
logging.basicConfig(
    level=logging.INFO,  # Set the minimum level to display (DEBUG, INFO, WARNING, ERROR, CRITICAL)
    format='%(asctime)s - %(filename)s:%(lineno)d - %(levelname)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

# --- Configuration (must match STM32 settings) ---
SPI_BUS = 0
SPI_DEVICE = 0
SPI_SPEED_HZ = 36000000  # Adjusted to match STM32's 36 MHz (PCLK2/2)
# SPI_MODE = 0b00  # CPOL=0, CPHA=0 (mode 0) - This is implied by default for spidev, but can be set explicitly if needed.

def display_menu():
    logging.info("\n--- Raspberry Pi Debug Menu ---")
    logging.info("1. Receive STM32 status (default)")
    logging.info("2. Send command: Toggle LED (Example)")
    logging.info("3. Send command: Request specific sensor data (Example)")
    logging.info("Q. Quit")
    return input("Enter your choice: ").strip().upper()

def main():
    logging.info("Raspberry Pi SPI Slave Application")
    spi_slave = None
    try:
        spi_slave = SPISlave(bus=SPI_BUS, device=SPI_DEVICE, max_speed_hz=SPI_SPEED_HZ)
        logging.info(f"SPI connection opened on bus {spi_slave.bus}, device {spi_slave.device}.")

        expected_data_size = struct.calcsize(FLAT_MOWER_STATUS_FORMAT)
        logging.info(f"Expecting to receive {expected_data_size} bytes per message.")

        while True:
            choice = display_menu()

            if choice == 'Q':
                break
            elif choice == '2':
                logging.info("Sending Toggle LED command...")
                # Example command: a single byte for command ID
                response = spi_slave.send_command(b'\x01', rx_len=1) # Send 0x01, expect 1 byte response
                logging.info(f"Command sent, STM32 response: {response.hex()}")
            elif choice == '3':
                logging.info("Sending Request Sensor Data command...")
                # Example command: command ID + sensor ID
                response = spi_slave.send_command(b'\x02\x01', rx_len=5) # Request sensor 0x01, expect 5 bytes
                logging.info(f"Command sent, STM32 response: {response.hex()}")
            elif choice == '1':
                logging.info("Waiting for data from STM32...")
                received_raw_data = spi_slave.receive_data(expected_data_size)

                if received_raw_data:
                    try:
                        lawn_mower_status = FlatLawnMowerStatus.from_bytes(received_raw_data)
                        logging.info(f"Received data: {lawn_mower_status}")
                    except struct.error as e:
                        logging.error(f"Error unpacking received data: {e}")
                        logging.error(f"Raw data: {received_raw_data.hex()}")
                else:
                    logging.info("No data received.")
            else:
                logging.warning("Invalid choice. Please try again.")

            time.sleep(0.1)

    except Exception as e:
        logging.critical(f"An error occurred: {e}")
    finally:
        if spi_slave:
            spi_slave.close()
            logging.info("SPI connection closed.")

if __name__ == "__main__":
    main()
