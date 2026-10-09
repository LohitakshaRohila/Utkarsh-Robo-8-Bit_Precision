import socket
import time
import sys
import termios
import tty
import select

ESP_IP = "172.30.82.134"
PORT = 4210

RATE_HZ = 20

THROTTLE_SPEED = 70
STEER_AMOUNT = 50

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def get_key():
    """Read a key without waiting for Enter."""
    if select.select([sys.stdin], [], [], 0)[0]:
        return sys.stdin.read(1).lower()
    return None

old_settings = termios.tcgetattr(sys.stdin)
last_packet = ""
throttle = 0
steering = 0

print("\n=== RC Car Terminal Controller ===")
print("W = Forward | S = Reverse")
print("A = Left    | D = Right")
print("X = Stop    | Q = Quit")
print("Use combinations like W+A to move forward-left.")
print("==================================\n")

try:
    tty.setcbreak(sys.stdin.fileno())

    while True:
        key = get_key()

        if key == "q":
            break

        if key == "w":
            throttle = THROTTLE_SPEED
        elif key == "s":
            throttle = -THROTTLE_SPEED
        elif key == "a":
            steering = -STEER_AMOUNT
        elif key == "d":
            steering = STEER_AMOUNT
        elif key == "x":
            throttle = 0
            steering = 0

        packet = f"T:{throttle},S:{steering}"

        sock.sendto(packet.encode(), (ESP_IP, PORT))

        if packet != last_packet:
            print(f"\rThrottle: {throttle:>4} | Steering: {steering:>4}   ")
            last_packet = packet

        time.sleep(1 / RATE_HZ)

finally:
    termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)

    for _ in range(3):
        sock.sendto(b"STOP", (ESP_IP, PORT))
        time.sleep(0.02)

    sock.close()
    print("\nRC controller stopped.")
