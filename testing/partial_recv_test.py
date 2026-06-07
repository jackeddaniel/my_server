import socket
import time

HOST = '127.0.0.1'
PORT = 3490

part1 = "GET /index.html HTTP/1.1\r\n"
part2 = (
    "Host: localhost:3490\r\n"
    "User-Agent: fragtest/1.0\r\n"
    "Accept: */*\r\n"
    "\r\n"
)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
sock.connect((HOST, PORT))

sock.sendall(part1.encode())
print(f"Sent part 1: {repr(part1)}")

time.sleep(5)  # longer than server's sleep(3), so recv fires on just part1

sock.sendall(part2.encode())
print(f"Sent part 2: {repr(part2)}")

try:
    response = sock.recv(4096)
    print("--- Response ---")
    print(response.decode(errors='replace'))
except Exception as e:
    print(f"Error receiving response: {e}")

sock.close()
