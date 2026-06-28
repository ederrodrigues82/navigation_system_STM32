#!/usr/bin/env python3
"""Right wheel motor + encoder test suite (Phases 1-4) over Pi UART."""

import argparse
import os
import sys
import time

from uart_client import UARTClient
from main import (
    DIR_BACKWARD,
    DIR_FORWARD,
    DIR_STOP,
    WHEEL_RIGHT,
    WHEEL_STATUS_READY,
    WHEEL_STATUS_NAMES,
    request_move,
    request_ping,
    request_status,
)

# Firmware maps right-wheel encoder count to left_encoder_count in status.
RIGHT_WHEEL_ENCODER = "left_encoder_count"
RIGHT_WHEEL_STATUS_IDX = 0

UART_PORT = os.environ.get("UART_PORT", "/dev/ttyS0")
UART_BAUDRATE = 115200
PULSE_TOLERANCE = 2
MOVE_TIMEOUT_S = 10.0
STATUS_POLL_S = 0.2


def _right_encoder_count(status) -> int:
    return getattr(status, RIGHT_WHEEL_ENCODER)


def _right_wheel_status(status) -> int:
    return status.wheel_status[RIGHT_WHEEL_STATUS_IDX]


def _status_name(code: int) -> str:
    if code < len(WHEEL_STATUS_NAMES):
        return WHEEL_STATUS_NAMES[code]
    return f"UNK({code})"


def _pass(msg: str) -> None:
    print(f"  PASS: {msg}")


def _fail(msg: str) -> None:
    print(f"  FAIL: {msg}")


def phase1_ping(client: UARTClient) -> bool:
    print("\n=== Phase 1: UART link (CMD_PING) ===")
    ok = request_ping(client)
    if ok:
        _pass("Received PING response from STM32")
    else:
        _fail("No PING response (check wiring, port, baud rate)")
    return ok


def phase2_encoder_manual(client: UARTClient, interactive: bool) -> bool:
    print("\n=== Phase 2: Encoder-only (manual wheel rotation) ===")
    print("  Right encoder: A0 (PA0) + A4 (PA4)")
    if interactive:
        input("Rotate the RIGHT wheel by hand (~10 turns), then press Enter...")

    samples = []
    for _ in range(25):
        status = request_status(client)
        if status:
            samples.append(_right_encoder_count(status))
        time.sleep(STATUS_POLL_S)

    if len(samples) < 2:
        _fail("Could not read encoder status")
        return False

    delta = samples[-1] - samples[0]
    print(f"  Encoder samples: first={samples[0]}, last={samples[-1]}, delta={delta}")
    if interactive and delta == 0:
        _fail("Encoder count did not change (check PA0/PA4 wiring and TIM2)")
        return False
    if not interactive and delta == 0:
        print("  SKIP: non-interactive mode; encoder delta is 0 (spin wheel manually to verify)")
        return True

    _pass(f"Encoder count changed by {delta} pulses")
    return True


def phase3_motor_open_loop(client: UARTClient, interactive: bool) -> bool:
    print("\n=== Phase 3: Motor open-loop (forward / backward / stop) ===")
    if interactive:
        print("  Observe the RIGHT wheel for 2 s in each direction.")

    request_move(client, WHEEL_RIGHT, DIR_FORWARD, 99999)
    time.sleep(2.0)
    request_move(client, WHEEL_RIGHT, DIR_STOP, 0)
    time.sleep(1.0)

    request_move(client, WHEEL_RIGHT, DIR_BACKWARD, 99999)
    time.sleep(2.0)
    request_move(client, WHEEL_RIGHT, DIR_STOP, 0)
    time.sleep(0.5)

    status = request_status(client)
    if status is None:
        _fail("Could not read status after motor test")
        return False

    if _right_wheel_status(status) != WHEEL_STATUS_READY:
        _fail(f"Wheel not READY after stop: {_status_name(_right_wheel_status(status))}")
        return False

    if interactive:
        answer = input("Did the wheel spin forward, then backward, then stop? [y/N]: ").strip().lower()
        if answer != "y":
            _fail("Operator reported motor direction/stop issue")
            return False

    _pass("Motor forward/backward/stop sequence completed; wheel status READY")
    return True


def _run_closed_loop_move(
    client: UARTClient,
    direction: int,
    target: int,
    label: str,
) -> bool:
    request_move(client, WHEEL_RIGHT, direction, target)

    saw_moving = False
    start = time.monotonic()
    while time.monotonic() - start < MOVE_TIMEOUT_S:
        status = request_status(client)
        if status is None:
            time.sleep(STATUS_POLL_S)
            continue

        ws = _right_wheel_status(status)
        if ws == 2:  # MOVING
            saw_moving = True
        if ws == WHEEL_STATUS_READY:
            break
        time.sleep(STATUS_POLL_S)

    status = request_status(client)
    request_move(client, WHEEL_RIGHT, DIR_STOP, 0)

    if status is None:
        _fail(f"{label}: no status after move")
        return False

    count = _right_encoder_count(status)
    ws = _right_wheel_status(status)
    err = abs(count - target)

    print(f"  {label}: target={target}, actual={count}, status={_status_name(ws)}, saw_moving={saw_moving}")

    if ws != WHEEL_STATUS_READY:
        _fail(f"{label}: expected READY, got {_status_name(ws)}")
        return False
    if err > PULSE_TOLERANCE:
        _fail(f"{label}: pulse count off by {err} (tolerance ±{PULSE_TOLERANCE})")
        return False
    if target > 0 and not saw_moving:
        _fail(f"{label}: never observed MOVING state")
        return False

    _pass(f"{label}: reached {count}/{target} pulses")
    return True


def phase4_closed_loop(client: UARTClient, targets: list[int]) -> bool:
    print("\n=== Phase 4: Closed-loop motor + encoder ===")
    all_ok = True
    for target in targets:
        if not _run_closed_loop_move(client, DIR_FORWARD, target, f"forward {target}"):
            all_ok = False
        time.sleep(0.5)
        if not _run_closed_loop_move(client, DIR_BACKWARD, target, f"backward {target}"):
            all_ok = False
        time.sleep(0.5)
    return all_ok


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Right wheel motor + encoder UART tests")
    parser.add_argument("--port", default=UART_PORT, help="Serial port (default: UART_PORT env or /dev/ttyS0)")
    parser.add_argument("--baud", type=int, default=UART_BAUDRATE, help="Baud rate")
    parser.add_argument(
        "--phase",
        type=int,
        choices=[1, 2, 3, 4],
        help="Run a single phase only",
    )
    parser.add_argument(
        "--non-interactive",
        action="store_true",
        help="Skip operator prompts (Phase 2/3 need manual observation)",
    )
    parser.add_argument(
        "--targets",
        type=int,
        nargs="+",
        default=[30, 100],
        help="Pulse targets for Phase 4 (default: 30 100)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    interactive = not args.non_interactive
    client = None
    results: dict[str, bool] = {}

    try:
        client = UARTClient(port=args.port, baudrate=args.baud)
        print(f"Opened {args.port} @ {args.baud} baud")

        phases = {
            1: lambda: phase1_ping(client),
            2: lambda: phase2_encoder_manual(client, interactive),
            3: lambda: phase3_motor_open_loop(client, interactive),
            4: lambda: phase4_closed_loop(client, args.targets),
        }

        to_run = [args.phase] if args.phase else [1, 2, 3, 4]
        for n in to_run:
            results[f"phase{n}"] = phases[n]()
            if not results[f"phase{n}"] and n < 4 and args.phase is None:
                print(f"\nStopping early after Phase {n} failure.")
                break

    except KeyboardInterrupt:
        print("\nInterrupted.")
        return 130
    except Exception as exc:
        print(f"\nError: {exc}")
        return 1
    finally:
        if client:
            try:
                request_move(client, WHEEL_RIGHT, DIR_STOP, 0)
            except Exception:
                pass
            client.close()

    print("\n=== Summary ===")
    for name, ok in results.items():
        print(f"  {name}: {'PASS' if ok else 'FAIL'}")

    return 0 if results and all(results.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
