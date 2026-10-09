import socket
import sys
import time
import termios
import tty
import select

# ========================================
# ESP32 CONFIGURATION
# ========================================

ESP_IP = "172.30.82.134"  # Replace with the IP printed by your ESP32
PORT = 4210

RATE_HZ = 20
SEND_INTERVAL = 1.0 / RATE_HZ

# ========================================
# MOVEMENT SETTINGS
# ========================================

THROTTLE_SPEED = 70
STEER_AMOUNT = 50

# ========================================
# UDP SETUP
# ========================================

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ESP_ADDRESS = (ESP_IP, PORT)

# ========================================
# TERMINAL KEYBOARD INPUT
# ========================================

def get_key():
    readable, _, _ = select.select([sys.stdin], [], [], 0)

    if readable:
        return sys.stdin.read(1).lower()

    return None


def send_command(throttle, steering):
    message = f"T:{throttle},S:{steering}"
    sock.sendto(message.encode(), ESP_ADDRESS)


def main():

    old_terminal_settings = termios.tcgetattr(sys.stdin)

    throttle = 0
    steering = 0

    last_sent = 0.0

    print("\n========== RC CAR CONTROLLER ==========")
    print("W : Forward")
    print("S : Reverse")
    print("A : Turn left")
    print("D : Turn right")
    print("SPACE : Stop")
    print("Q : Quit")
    print("=======================================\n")
    print("Starting controller...\n")

    try:
        tty.setcbreak(sys.stdin.fileno())

        while True:

            key = get_key()

            if key == "q":
                break

            elif key == "w":
                throttle = THROTTLE_SPEED
                steering = 0

            elif key == "s":
                throttle = -THROTTLE_SPEED
                steering = 0

            elif key == "a":
                throttle = 0
                steering = -STEER_AMOUNT

            elif key == "d":
                throttle = 0
                steering = STEER_AMOUNT

            elif key == " ":
                throttle = 0
                steering = 0

            now = time.monotonic()

            if now - last_sent >= SEND_INTERVAL:
                send_command(throttle, steering)
                last_sent = now

                print(
                    f"\rThrottle: {throttle:4d} | "
                    f"Steering: {steering:4d}    ",
                    end="",
                    flush=True
                )

            time.sleep(0.002)

    except KeyboardInterrupt:
        print("\nInterrupted.")

    finally:
        termios.tcsetattr(
            sys.stdin,
            termios.TCSADRAIN,
            old_terminal_settings
        )

        sock.close()
        print("\nController exited.")


if __name__ == "__main__":
    main()
