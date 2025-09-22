import socket
import struct
import threading
import time
import random

# Build a 16-byte dummy payload for GET requests
def make_random_topology():
  # 3 address in one entry
  addr_template = [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F]
  addr_template += [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F]
  addr_template += [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F]
  res = bytearray()
  for i in range(random.randint(0, 15)):
    entry = [addr_template[j] + i * 0x10 for j in range(len(addr_template))]
    res.extend(entry)
  return bytes(res)

# Build settings payload for SET_CONFIG_PARAMS, matching C struct
def build_settings_payload():
  network_name = b"Test_Network" + b"\x00" * (33 - len("Test_Network"))
  regulatory_domain = 1
  network_size = 0
  chan_plan_id = 0
  phy_mode_id = 0
  tx_power_ddbm = 0
  pan_id = 0x1234
  gaks = bytes([0] * (4 * 16))
  gtks = bytes([
    0xBB, 0x06, 0x08, 0x57, 0x2C, 0xE1, 0x4D, 0x7B, 0xA2, 0xD1, 0x55, 0x49, 0x9C, 0xC8, 0x51, 0x9B,
    *([0] * 16),
    *([0] * 16),
    *([0] * 16)
  ])
  # struct format: 33s 5B h H 64s 64s
  payload = struct.pack(
    "<33sBBBBhH64s64s",
    network_name,
    regulatory_domain,
    network_size,
    chan_plan_id,
    phy_mode_id,
    tx_power_ddbm,
    pan_id,
    gaks,
    gtks
  )
  return payload

APP_HOST = "::1"
APP_PORT = 5678
BOT_LISTEN_HOST = "::"
BOT_LISTEN_PORT = 6789
SEND_INTERVAL = 5


def build_message(msg_code, payload=b""):
  payload_len = len(payload)
  return struct.pack("!II", msg_code, payload_len) + payload



def send_periodic_requests():
  while True:
    try:
      with socket.socket(socket.AF_INET6, socket.SOCK_STREAM) as sock:
        sock.connect((APP_HOST, APP_PORT))
        # Example: send TOPOLOGY (code 0x01) with 16-byte dummy payload
        topolgoy_payload = make_random_topology()
        msg_get = build_message(0x01, topolgoy_payload)
        sock.sendall(msg_get)
        print(f"Sent TOPOLOGY: code=0x01, len={len(topolgoy_payload)} payload={topolgoy_payload.hex()}")
        response = sock.recv(2048)
        print(f"Received {len(response)} bytes from app (GET)")
    except Exception as e:
      print(f"Send error: {e}")
    time.sleep(SEND_INTERVAL)

def tcp_server():
  with socket.socket(socket.AF_INET6, socket.SOCK_STREAM) as server:
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((BOT_LISTEN_HOST, BOT_LISTEN_PORT))
    server.listen(5)
    print(f"Bot TCP server listening on [{BOT_LISTEN_HOST}]:{BOT_LISTEN_PORT}")
    while True:
      conn, addr = server.accept()
      with conn:
        print(f"Accepted connection from {addr}")
        data = conn.recv(4096)
        if data:
          print(f"Received {len(data)} bytes from app")
          print_msg_like_utils(data)

def print_msg_like_utils(data):
  if len(data) < 8:
    print("Invalid message (too short)")
    return
  msg_code = int.from_bytes(data[0:4], byteorder='big')
  payload_len = int.from_bytes(data[4:8], byteorder='big')
  payload = data[8:8+payload_len]
  code_str = {
    0x01: "TOPOLOGY",
    0x02: "GET_CONFIG_PARAMS",
    0x03: "SET_CONFIG_PARAMS",
    0x04: "START_BR",
    0x05: "STOP_BR"
  }.get(msg_code, "UNKNOWN")
  print(f"Msg code: {msg_code:04x} ({code_str})")
  print(f"Payload len: {payload_len}")
  if payload_len:
    print("Payload data:")
    for i in range(payload_len):
      if i % 16 == 0:
        print(f"\n{i:04x}: ", end='')
      print(f"{payload[i]:02x} ", end='')
    print()

if __name__ == "__main__":
  t1 = threading.Thread(target=send_periodic_requests, daemon=True)
  t2 = threading.Thread(target=tcp_server, daemon=True)
  t1.start()
  t2.start()
  print("Bot started. Press Ctrl+C to exit.")
  try:
    while True:
      time.sleep(1)
  except KeyboardInterrupt:
    print("Bot stopped.")
  