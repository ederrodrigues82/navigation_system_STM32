#!/usr/bin/env python3
"""Wheel sensor emulation test suite over existing Pi UART (USART2)."""

import argparse
import os
import sys
import time

from uart_client import UARTClient
from main import (
    DIR_BACKWARD,
    DIR_FORWARD,
    DIR_STOP,
    WHEEL_BOTH,
    WHEEL_LEFT,
    WHEEL_RIGHT,
    WHEEL_STATUS_READY,
    WHEEL_STATUS_NAMES,
    request_move,
    request_ping,
    request_status,
    set_emulate_wheel,
)

# Firmware maps right-wheel encoder count to left_encoder_count in status.
RIGHT_WHEEL_ENCODER = "left_encoder_count"
RIGHT_WHEEL_STATUS_IDX = 0
LEFT_WHEEL_STATUS_IDX = 1

UART_PORT = os.environ.get("UART_PORT", "/dev/ttyS0")
UART_BAUDRATE = 115200
UART_READ_TIMEOUT = 2.0
PULSE_TOLERANCE = 2
EMULATE_PULSE_MS = 200
STATUS_POLL_S = 0.2


def _right_encoder_count(status) -> int:
    return getattr(status, RIGHT_WHEEL_ENCODER)


def _right_wheel_status(status) -> int:
    return status.wheel_status[RIGHT_WHEEL_STATUS_IDX]


def _left_wheel_status(status) -> int:
    return status.wheel_status[LEFT_WHEEL_STATUS_IDX]


def _status_name(code: int) -> str:
    if code < len(WHEEL_STATUS_NAMES):
        return WHEEL_STATUS_NAMES[code]
    return f"UNK({code})"


def _move_timeout_s(target: int) -> float:
    return target * (EMULATE_PULSE_MS / 1000.0) + 1.0


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


def phase2_emulate_toggle(client: UARTClient) -> bool:
    print("\n=== Phase 2: emulate_wheel toggle (CMD_SET_EMULATE_WHEEL) ===")
    on = set_emulate_wheel(client, True)
    if on != 1:
        _fail(f"Enable emulate failed (ACK={on}, expected 1)")
        return False
    _pass("emulate_wheel enabled (ACK=1)")

    off = set_emulate_wheel(client, False)
    if off != 0:
        _fail(f"Disable emulate failed (ACK={off}, expected 0)")
        return False
    _pass("emulate_wheel disabled (ACK=0)")
    return True


def phase3_right_forward(client: UARTClient, target: int = 5) -> bool:
    print(f"\n=== Phase 3: Right wheel forward ({target} pulses, emulated) ===")
    if set_emulate_wheel(client, True) != 1:
        _fail("Could not enable emulate_wheel")
        return False

    request_move(client, WHEEL_RIGHT, DIR_FORWARD, target)
    timeout = _move_timeout_s(target)
    start = time.monotonic()
    saw_moving = False

    while time.monotonic() - start < timeout:
        status = request_status(client)
        if status is None:
            time.sleep(STATUS_POLL_S)
            continue

        count = _right_encoder_count(status)
        ws = _right_wheel_status(status)
        if ws == 2:
            saw_moving = True
        print(f"  right forward: count={count}, status={_status_name(ws)}")
        if ws == WHEEL_STATUS_READY:
            break
        time.sleep(STATUS_POLL_S)

    elapsed = time.monotonic() - start
    status = request_status(client)
    if status is None:
        _fail("No status after forward move")
        return False

    count = _right_encoder_count(status)
    ws = _right_wheel_status(status)
    err = abs(count - target)

    print(f"  result: target={target}, actual={count}, elapsed={elapsed:.2f}s")

    if ws != WHEEL_STATUS_READY:
        _fail(f"Expected READY, got {_status_name(ws)}")
        return False
    if err > PULSE_TOLERANCE:
        _fail(f"Pulse count off by {err}")
        return False
    if not saw_moving:
        _fail("Never observed MOVING state")
        return False

    _pass(f"Right forward {count}/{target} pulses in {elapsed:.2f}s")
    return True


def phase4_right_backward(client: UARTClient, target: int = 3) -> bool:
    print(f"\n=== Phase 4: Right wheel backward ({target} pulses, emulated) ===")
    if set_emulate_wheel(client, True) != 1:
        _fail("Could not enable emulate_wheel")
        return False

    request_move(client, WHEEL_RIGHT, DIR_BACKWARD, target)
    timeout = _move_timeout_s(target)
    start = time.monotonic()
    saw_moving = False

    while time.monotonic() - start < timeout:
        status = request_status(client)
        if status is None:
            time.sleep(STATUS_POLL_S)
            continue

        count = _right_encoder_count(status)
        ws = _right_wheel_status(status)
        if ws == 2:
            saw_moving = True
        print(f"  right backward: count={count}, status={_status_name(ws)}")
        if ws == WHEEL_STATUS_READY:
            break
        time.sleep(STATUS_POLL_S)

    status = request_status(client)
    if status is None:
        _fail("No status after backward move")
        return False

    count = _right_encoder_count(status)
    ws = _right_wheel_status(status)
    err = abs(count - target)

    if ws != WHEEL_STATUS_READY:
        _fail(f"Expected READY, got {_status_name(ws)}")
        return False
    if err > PULSE_TOLERANCE:
        _fail(f"Pulse count off by {err}")
        return False
    if not saw_moving:
        _fail("Never observed MOVING state")
        return False

    _pass(f"Right backward {count}/{target} pulses")
    return True


def phase5_both_forward(client: UARTClient, target: int = 5) -> bool:
    print(f"\n=== Phase 5: Both wheels forward ({target} pulses, emulated) ===")
    if set_emulate_wheel(client, True) != 1:
        _fail("Could not enable emulate_wheel")
        return False

    request_move(client, WHEEL_BOTH, DIR_FORWARD, target)
    timeout = _move_timeout_s(target)
    start = time.monotonic()

    while time.monotonic() - start < timeout:
        status = request_status(client)
        if status is None:
            time.sleep(STATUS_POLL_S)
            continue

        left_count = status.left_encoder_count
        right_count = status.right_encoder_count
        left_ws = _left_wheel_status(status)
        right_ws = _right_wheel_status(status)
        print(
            f"  both forward: L={left_count}({_status_name(left_ws)}), "
            f"R={right_count}({_status_name(right_ws)})"
        )
        if left_ws == WHEEL_STATUS_READY and right_ws == WHEEL_STATUS_READY:
            break
        time.sleep(STATUS_POLL_S)

    status = request_status(client)
    if status is None:
        _fail("No status after both-wheels move")
        return False

    left_count = status.left_encoder_count
    right_count = status.right_encoder_count
    left_err = abs(left_count - target)
    right_err = abs(right_count - target)

    if _left_wheel_status(status) != WHEEL_STATUS_READY:
        _fail(f"Left wheel not READY: {_status_name(_left_wheel_status(status))}")
        return False
    if _right_wheel_status(status) != WHEEL_STATUS_READY:
        _fail(f"Right wheel not READY: {_status_name(_right_wheel_status(status))}")
        return False
    if left_err > PULSE_TOLERANCE:
        _fail(f"Left encoder off by {left_err} (count={left_count}, target={target})")
        return False
    if right_err > PULSE_TOLERANCE:
        _fail(f"Right encoder off by {right_err} (count={right_count}, target={target})")
        return False

    _pass(f"Both wheels reached L={left_count} R={right_count} (target={target})")
    return True


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Wheel sensor emulation UART tests")
    parser.add_argument("--port", default=UART_PORT, help="Serial port (default: UART_PORT env or /dev/ttyS0)")
    parser.add_argument("--baud", type=int, default=UART_BAUDRATE, help="Baud rate")
    parser.add_argument(
        "--phase",
        type=int,
        choices=[1, 2, 3, 4, 5],
        help="Run a single phase only",
    )
    parser.add_argument(
        "--non-interactive",
        action="store_true",
        help="Non-interactive mode (no prompts)",
    )
    parser.add_argument(
        "--target",
        type=int,
        default=5,
        help="Pulse target for phases 3-5 (default: 5)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    client = None
    results: dict[str, bool] = {}

    try:
        client = UARTClient(port=args.port, baudrate=args.baud, timeout=UART_READ_TIMEOUT)
        print(f"Opened {args.port} @ {args.baud} baud")

        phases = {
            1: lambda: phase1_ping(client),
            2: lambda: phase2_emulate_toggle(client),
            3: lambda: phase3_right_forward(client, args.target),
            4: lambda: phase4_right_backward(client, min(args.target, 3)),
            5: lambda: phase5_both_forward(client, args.target),
        }

        to_run = [args.phase] if args.phase else [1, 2, 3, 4, 5]
        for n in to_run:
            results[f"phase{n}"] = phases[n]()
            if not results[f"phase{n}"] and n < max(to_run) and args.phase is None:
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
                set_emulate_wheel(client, False)
                request_move(client, WHEEL_BOTH, DIR_STOP, 0)
            except Exception:
                pass
            client.close()

    print("\n=== Summary ===")
    for name, ok in results.items():
        print(f"  {name}: {'PASS' if ok else 'FAIL'}")

    return 0 if results and all(results.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
