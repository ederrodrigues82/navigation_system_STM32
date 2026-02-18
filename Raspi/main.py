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

from rich.console import Group
from rich.live import Live
from rich.panel import Panel
from rich.table import Table

from uart_client import UARTClient
from flat_lawn_mower_status import FlatLawnMowerStatus, STATUS_STRUCT_SIZE

# Configure logging (WARNING to avoid cluttering Rich display; use INFO for debug)
logging.basicConfig(
    level=logging.WARNING,
    format='%(asctime)s - %(filename)s:%(lineno)d - %(levelname)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

# --- Configuration (must match STM32 USART2 settings) ---
UART_PORT = "/dev/ttyS0"  # Pi Zero W (use /dev/ttyAMA0 for Pi 3/4/5)
UART_BAUDRATE = 115200

# --- UART command bytes (must match STM32 communication_test.h) ---
CMD_PING = 0x01
CMD_STATUS = 0x02


def request_ping(uart_client: UARTClient) -> bool:
    """Send PING command, receive 4-byte response. Returns True if response received."""
    try:
        uart_client.send_data(bytes([CMD_PING]))
        uart_client.serial.flush()
        received = uart_client.receive_data(4)
        if received and len(received) == 4:
            logging.info(f"PING response: {received}")
            return True
        logging.warning("No PING response from STM32")
        return False
    except Exception as e:
        logging.error(f"PING failed: {e}")
        logging.critical(traceback.format_exc())
        return False


def make_status_display(status: FlatLawnMowerStatus | None) -> Panel:
    """Build Rich display for m_status. Returns placeholder if status is None."""
    if status is None:
        return Panel("[yellow]Waiting for STM32...[/yellow]", title="Mower Status")

    def add_table(title: str, rows: list[tuple[str, str]]) -> Table:
        t = Table(show_header=False, box=None, padding=(0, 1))
        t.add_column(style="cyan")
        t.add_column(style="green")
        for k, v in rows:
            t.add_row(k, v)
        return Panel(t, title=title, border_style="blue")

    motion = add_table("Motion", [
        ("Motors", f"L: {status.left_motor_speed}  R: {status.right_motor_speed}"),
        ("Direction", status.direction or "(none)"),
        ("Encoders", f"L: {status.left_encoder_count}  R: {status.right_encoder_count}"),
        ("Speed", f"{status.speed_mps:.2f} m/s"),
        ("Heading", f"{status.heading_deg:.1f}°"),
    ])

    position = add_table("Position", [
        ("Pos (x,y,z)", f"({status.pos_x:.1f}, {status.pos_y:.1f}, {status.pos_z:.1f})"),
        ("Euler (r,p,y)", f"({status.euler_roll:.1f}, {status.euler_pitch:.1f}, {status.euler_yaw:.1f})"),
    ])

    sensors = add_table("Sensors", [
        ("Bumpers", str(list(status.bumpers))),
        ("IRDA (cm)", str(list(status.irda_distance))),
        ("Rain", "Yes" if status.rain_detected else "No"),
        ("Blade", f"{'ON' if status.blade_motor_status else 'OFF'} @ {status.blade_speed_rpm} RPM"),
    ])

    battery = add_table("Battery", [
        ("Voltage", f"{status.battery_voltage:.2f} V"),
        ("Current", f"{status.battery_current:.2f} A"),
        ("Charge", f"{status.battery_percentage}%"),
        ("Charging", "Yes" if status.charging_status else "No"),
    ])

    system = add_table("System", [
        ("Uptime", f"{status.uptime_ms} ms"),
        ("Error", hex(status.error_code)),
        ("Manual", "Yes" if status.is_manual_mode else "No"),
        ("E-Stop", "Yes" if status.is_emergency_stop else "No"),
        ("Task State", str(status.task_state)),
        ("Edge Sensor", str(status.edge_sensor)),
    ])

    return Panel(
        Group(motion, position, sensors, battery, system),
        title="[bold]Mower Status[/bold]",
        border_style="green",
    )


def request_status(uart_client: UARTClient) -> FlatLawnMowerStatus | None:
    """Send STATUS command, receive flat_lawn_mower_status struct. Returns None on failure."""
    try:
        uart_client.send_data(bytes([CMD_STATUS]))
        uart_client.serial.flush()
        received = uart_client.receive_data(STATUS_STRUCT_SIZE)
        if received and len(received) == STATUS_STRUCT_SIZE:
            status = FlatLawnMowerStatus.from_bytes(received)
            return status
        logging.warning(f"Incomplete status: got {len(received) if received else 0} bytes, expected {STATUS_STRUCT_SIZE}")
        return None
    except Exception as e:
        logging.error(f"Status request failed: {e}")
        logging.critical(traceback.format_exc())
        return None


# --- Main application logic ---
# The interactive menu logic is removed for this simplified ping test

if __name__ == "__main__":
    uart_client = None
    try:
        uart_client = UARTClient(port=UART_PORT, baudrate=UART_BAUDRATE)
        refresh_interval = 0.5  # seconds between status requests

        with Live(make_status_display(None), refresh_per_second=4, console=None) as live:
            while True:
                status = request_status(uart_client)
                live.update(make_status_display(status))
                time.sleep(refresh_interval)

    except KeyboardInterrupt:
        pass
    except Exception as e:
        logging.critical(f"An error occurred: {e}")
        logging.critical(traceback.format_exc())
    finally:
        if uart_client:
            uart_client.close()
