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
import traceback # Import traceback for detailed error logging

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

# --- Function to handle SPI communication (Simplified for Ping Test) ---
def receive_status_from_stm32(spi_slave: SPISlave, expected_message_size: int):
    logging.info("Waiting for data from STM32...")
    try:
        # Receive data from STM32 (assuming STM32 is master)
        # We expect a certain number of bytes based on the struct size (4 for "PING")
        received_data = spi_slave.receive_data(expected_message_size)
        
        # Log raw received data, but only if it's not all zeros
        if received_data != b'\x00' * expected_message_size:
            logging.info(f"Raw received data: {received_data}")

        # Commenting out the deserialization for simple ping test
        # mower_status = FlatLawnMowerStatus.from_bytes(bytes(received_data))
        # logging.info(f"Received Mower Status: {mower_status}")

    except Exception as e:
        logging.error(f"Error receiving data from STM32: {e}")
        logging.critical(traceback.format_exc())


# --- Main application logic ---
# The interactive menu logic is removed for this simplified ping test

if __name__ == "__main__":
    try:
        # Initialize SPI as slave
        spi_slave = SPISlave(bus=0, device=0)
        #spi_slave.open_spi()
        logging.info(f"SPI connection opened on bus {spi_slave.bus}, device {spi_slave.device}.")
        
        # Define the expected message size for the ping test (4 bytes for "PING")
        # Make sure this matches the STM32's SPI_PING_MESSAGE_SIZE
        expected_message_size = 4 
        logging.info(f"Expecting to receive {expected_message_size} bytes per message (for PING test).")

        while True:
            receive_status_from_stm32(spi_slave, expected_message_size)
            time.sleep(0.1) # Small delay to prevent busy-waiting

    except Exception as e:
        logging.critical(f"An error occurred: {e}")
        logging.critical(traceback.format_exc())
    finally:
        logging.info("SPI connection closed.")
        if spi_slave:
            spi_slave.close()
