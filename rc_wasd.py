import socket
import pygame
import time

ESP_IP = "172.30.82.134"
PORT = 4210

RATE_HZ = 20

THROTTLE_SPEED = 70
STEER_AMOUNT = 50

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

pygame.init()

screen = pygame.display.set_mode((400, 150))
pygame.display.set_caption("RC Car - WASD to Drive")

clock = pygame.time.Clock()

running = True
last_packet = ""

try:
    while running:

        for event in pygame.event.get():

            if event.type == pygame.QUIT:
                running = False

        keys = pygame.key.get_pressed()

        throttle = 0
        steering = 0

        # Forward and reverse
        if keys[pygame.K_w] and not keys[pygame.K_s]:
            throttle = THROTTLE_SPEED

        elif keys[pygame.K_s] and not keys[pygame.K_w]:
            throttle = -THROTTLE_SPEED

        # Left and right
        if keys[pygame.K_a] and not keys[pygame.K_d]:
            steering = -STEER_AMOUNT

        elif keys[pygame.K_d] and not keys[pygame.K_a]:
            steering = STEER_AMOUNT

        packet = f"T:{throttle},S:{steering}"

        # Send commands continuously.
        sock.sendto(packet.encode(), (ESP_IP, PORT))

        # Print only when the command changes.
        if packet != last_packet:
            print("Sending:", packet)
            last_packet = packet

        # Update the controller display.
        screen.fill((30, 30, 30))

        font = pygame.font.Font(None, 30)
        text = font.render(
            f"Throttle: {throttle} | Steering: {steering}",
            True,
            (255, 255, 255)
        )

        screen.blit(text, (15, 50))

        pygame.display.flip()

        clock.tick(RATE_HZ)

finally:
    # Send several stop packets in case one is lost.
    for _ in range(3):
        sock.sendto(b"STOP", (ESP_IP, PORT))
        time.sleep(0.02)

    sock.close()
    pygame.quit()