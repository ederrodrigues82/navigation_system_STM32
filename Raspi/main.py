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
UART_PORT = "/dev/ttyS0"  # Pi Zero W (use /dev/ttyAMA0 for Pi 3/4/5)
UART_BAUDRATE = 115200

# --- Function to handle UART communication (RPi initiates, STM32 responds) ---
PING_REQUEST = b"PING"

def request_status_from_stm32(uart_client: UARTClient, expected_response_size: int):
    """RPi sends PING request, then receives STM32 response."""
    try:
        # RPi initiates: send request to STM32
        uart_client.send_data(PING_REQUEST)
        uart_client.serial.flush()
        logging.info("Sent PING request to STM32, waiting for response...")

        # Receive response from STM32
        received_data = uart_client.receive_data(expected_response_size)

        if received_data:
            logging.info(f"Received from STM32: {received_data}")
        else:
            logging.warning("No response from STM32 (timeout or empty)")

    except Exception as e:
        logging.error(f"Error communicating with STM32: {e}")
        logging.critical(traceback.format_exc())


# --- Main application logic ---
# The interactive menu logic is removed for this simplified ping test

if __name__ == "__main__":
    uart_client = None
    try:
        # Initialize UART (RPi initiates communication, STM32 responds)
        uart_client = UARTClient(port=UART_PORT, baudrate=UART_BAUDRATE)
        logging.info(f"UART connection opened on {uart_client.port} at {uart_client.baudrate} baud.")

        # RPi sends PING (4 bytes), STM32 responds with PING (4 bytes)
        expected_response_size = 4
        logging.info("RPi initiates: sending PING request, expecting 4-byte response.")

        while True:
            request_status_from_stm32(uart_client, expected_response_size)
            time.sleep(0.5)  # Interval between requests

    except Exception as e:
        logging.critical(f"An error occurred: {e}")
        logging.critical(traceback.format_exc())
    finally:
        logging.info("UART connection closed.")
        if uart_client:
            uart_client.close()
