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
import struct
import traceback  # Import traceback for detailed error logging

from rich.console import Group
from rich.live import Live
from rich.panel import Panel
from rich.table import Table

from uart_client import UARTClient
from flat_lawn_mower_status import FlatLawnMowerStatus, STATUS_STRUCT_SIZE, WHEEL_STATUS_NAMES, WHEEL_STATUS_READY

# Configure logging (WARNING to avoid cluttering Rich display; use INFO for debug)
logging.basicConfig(
    level=logging.WARNING,
    format='%(asctime)s - %(filename)s:%(lineno)d - %(levelname)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

# --- Configuration (must match STM32 USART2 settings) ---
UART_PORT = "/dev/ttyS0"  # Pi Zero W (use /dev/ttyAMA0 for Pi 3/4/5)
UART_BAUDRATE = 115200
UART_READ_TIMEOUT = 2.0
# STM32 finishes USART2 handler before Pi sends next frame (avoids RX desync)
UART_POST_CMD_DELAY_S = 0.05
UART_POST_MOVE_DELAY_S = 0.25

# --- UART command bytes (must match STM32 communication_test.h) ---
CMD_PING = 0x01
CMD_STATUS = 0x02
CMD_MOVE = 0x03
CMD_SET_EMULATE_WHEEL = 0x04

# Wheel selection (must match STM32 actuators.h)
WHEEL_RIGHT = 0
WHEEL_LEFT = 1
WHEEL_BOTH = 2

# Direction (must match Movement_direction enum in actuators.h)
DIR_FORWARD = 0
DIR_BACKWARD = 1
DIR_STOP = 2
DIR_ROTATING_CLOCK = 3
DIR_ROTATING_COUNTER = 4


def _uart_prepare(uart_client: UARTClient) -> None:
    """Discard stale RX bytes before a new command/response pair."""
    uart_client.flush_input()


def set_emulate_wheel(uart_client: UARTClient, enable: bool) -> int:
    """Enable or disable wheel sensor emulation on STM32. Returns active mode (0/1) or -1 on failure."""
    try:
        _uart_prepare(uart_client)
        uart_client.send_data(bytes([CMD_SET_EMULATE_WHEEL, 1 if enable else 0]))
        uart_client.serial.flush()
        ack = uart_client.receive_data(1)
        time.sleep(UART_POST_CMD_DELAY_S)
        if len(ack) == 1:
            logging.info(f"emulate_wheel set to {ack[0]}")
            return ack[0]
        logging.warning("No ACK from CMD_SET_EMULATE_WHEEL")
        return -1
    except Exception as e:
        logging.error(f"set_emulate_wheel failed: {e}")
        logging.critical(traceback.format_exc())
        return -1


def request_move(uart_client: UARTClient, wheel: int, direction: int, distance: int) -> None:
    """Send movement command to STM32. Fire-and-forget, no response expected."""
    _uart_prepare(uart_client)
    payload = bytes([CMD_MOVE, wheel, direction]) + struct.pack("<i", distance)
    uart_client.send_data(payload)
    uart_client.serial.flush()
    time.sleep(UART_POST_MOVE_DELAY_S)


def request_ping(uart_client: UARTClient) -> bool:
    """Send PING command, receive 4-byte response. Returns True if response received."""
    try:
        _uart_prepare(uart_client)
        uart_client.send_data(bytes([CMD_PING]))
        uart_client.serial.flush()
        received = uart_client.receive_data(4)
        time.sleep(UART_POST_CMD_DELAY_S)
        if len(received) == 4:
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

    def wheel_status_str(ws: tuple) -> str:
        names = [WHEEL_STATUS_NAMES[s] if s < len(WHEEL_STATUS_NAMES) else f"UNK({s})" for s in ws]
        return f"L: {names[0]}, R: {names[1]}"

    motion = add_table("Motion", [
        ("Motors", f"L: {status.left_motor_speed}  R: {status.right_motor_speed}"),
        ("Direction", status.direction or "(none)"),
        ("Wheel Status", wheel_status_str(status.wheel_status)),
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
        _uart_prepare(uart_client)
        uart_client.send_data(bytes([CMD_STATUS]))
        uart_client.serial.flush()
        received = uart_client.receive_data(STATUS_STRUCT_SIZE)
        time.sleep(UART_POST_CMD_DELAY_S)
        if len(received) == STATUS_STRUCT_SIZE:
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

def run_move_test(uart_client: UARTClient) -> None:
    """Test: move both wheels 30 pulses forward, wait for READY, then stop."""
    print("Test: moving both wheels 30 pulses forward...")
    request_move(uart_client, WHEEL_BOTH, DIR_FORWARD, 30)
    timeout = 10.0
    start = time.monotonic()
    while time.monotonic() - start < timeout:
        status = request_status(uart_client)
        if status and all(s == WHEEL_STATUS_READY for s in status.wheel_status):
            break
        time.sleep(0.2)
    print("Test: stopping both wheels...")
    request_move(uart_client, WHEEL_BOTH, DIR_STOP, 0)
    time.sleep(0.5)
    print("Test done.")


if __name__ == "__main__":
    uart_client = None
    try:
        uart_client = UARTClient(port=UART_PORT, baudrate=UART_BAUDRATE, timeout=UART_READ_TIMEOUT)
        run_move_test(uart_client)
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
