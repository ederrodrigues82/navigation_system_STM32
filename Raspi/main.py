# Raspi/main.py
import os
import time

# Optional: wait for debugger when DEBUG_UART=1 (e.g. DEBUG_UART=1 python main.py)
if os.environ.get("DEBUG_UART") == "1":
    import debugpy
    debugpy.listen(("0.0.0.0", 5678))
    print("Waiting for debugger client to attach...")
    debugpy.wait_for_client()
    print("Debugger attached! Continuing execution.")
import logging
import traceback  # Import traceback for detailed error logging

from uart_client import UARTClient

# Configure logging
logging.basicConfig(
    level=logging.INFO,  # Set the minimum level to display (DEBUG, INFO, WARNING, ERROR, CRITICAL)
    format='%(asctime)s - %(filename)s:%(lineno)d - %(levelname)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

# --- Configuration (must match STM32 USART2 settings) ---
UART_PORT = "/dev/ttyAMA0"  # Use /dev/ttyS0 for Pi Zero W
UART_BAUDRATE = 115200

# --- Function to handle UART communication (Simplified for Ping Test) ---
def receive_status_from_stm32(uart_client: UARTClient, expected_message_size: int):
    logging.info("Waiting for data from STM32...")
    try:
        # RPi receives data sent by STM32 over UART
        received_data = uart_client.receive_data(expected_message_size)

        # Log raw received data, but only if it's not empty
        if received_data:
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
    uart_client = None
    try:
        # Initialize UART (RPi receives from STM32 USART2)
        uart_client = UARTClient(port=UART_PORT, baudrate=UART_BAUDRATE)
        logging.info(f"UART connection opened on {uart_client.port} at {uart_client.baudrate} baud.")

        # Define the expected message size for the ping test (4 bytes for "PING")
        # Make sure this matches the STM32's UART_PING_MESSAGE_SIZE
        expected_message_size = 4
        logging.info(f"Expecting to receive {expected_message_size} bytes per message (for PING test).")

        while True:
            receive_status_from_stm32(uart_client, expected_message_size)
            time.sleep(0.1)  # Small delay to prevent busy-waiting

    except Exception as e:
        logging.critical(f"An error occurred: {e}")
        logging.critical(traceback.format_exc())
    finally:
        logging.info("UART connection closed.")
        if uart_client:
            uart_client.close()
