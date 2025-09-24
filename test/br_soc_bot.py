import socket
import struct
import threading
import time
import random
import copy

CODE_STR_MAP = {
    0x01: "TOPOLOGY",
    0x02: "GET_CONFIG_PARAMS",
    0x03: "SET_CONFIG_PARAMS",
    0x04: "START_BR",
    0x05: "STOP_BR"
}

APP_HOST = "::1"
APP_PORT = 5678
BOT_LISTEN_HOST = "::"
BOT_LISTEN_PORT = 4567
SEND_INTERVAL = 5

def make_random_topology():
  # 3 address in one entry
  addr_template = [0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01]
  res = bytearray()
  
  num_topology_entry = random.randint(1, 32)
  # BR always ..0x01
  res.extend(addr_template)
  res.extend([0x00] * 16)
  res.extend([0x00] * 16)
  
  parent =copy.deepcopy(addr_template)
  for i in range(1, num_topology_entry):
    addr_template[15] = i
    res.extend(addr_template)
    
    res.extend(parent)
    res.extend([0x00] * 16)
    
    # random parent update
    if  bool(random.getrandbits(1)):
      parent = copy.deepcopy(addr_template)
    
  return bytes(res)

# Build settings payload for SET_CONFIG_PARAMS, matching C struct
def build_settings_payload():
  # # network_name (33 bytes, ASCII, null-padded)
  # 57 69 2D 53 55 4E 5F 4E 65 74 77 6F 72 6B 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  # # regulatory_domain
  # 01
  # # network_size
  # 00
  # # chan_plan_id
  # 00
  # # phy_mode_id
  # 00
  # # tx_power_ddbm (int16, little-endian)
  # 00 00
  # # pan_id (uint16, little-endian)
  # 34 12
  # # gaks (4 x 16 bytes, all zeros)
  # 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  # 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  # 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  # 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  # # gtks (first 16 bytes as specified, rest zeros)
  # BB 06 08 57 2C E1 4D 7B A2 D1 55 49 9C C8 51 9B
  # 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  # 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  # 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  payload = bytes([
    # network_name: "Wi-SUN_Network" + null padding to 33 bytes
    0x57, 0x69, 0x2D, 0x53, 0x55, 0x4E, 0x5F, 0x4E, 0x65, 0x74, 0x77, 0x6F, 0x72, 0x6B,
    *([0x00] * (33 - 14)),
    # regulatory_domain
    0x01,
    # network_size
    0x00,
    # chan_plan_id
    0x00,
    # phy_mode_id
    0x00,
    # tx_power_ddbm (int16, little-endian)
    0x00, 0x00,
    # pan_id (uint16, little-endian)
    0x34, 0x12,
    # gaks (4 x 16 bytes, all zeros)
    *([0x00] * 64),
    # gtks (first 16 bytes as specified, rest zeros)
    0xBB, 0x06, 0x08, 0x57, 0x2C, 0xE1, 0x4D, 0x7B, 0xA2, 0xD1, 0x55, 0x49, 0x9C, 0xC8, 0x51, 0x9B,
    *([0x00] * 16),
    *([0x00] * 16),
    *([0x00] * 16)
  ])
  return payload


def build_message(msg_code, payload=b""):
  payload_len = len(payload)
  return struct.pack("!II", msg_code, payload_len) + payload

def send_request(msg_code, payload=b""):
  try:
    with socket.socket(socket.AF_INET6, socket.SOCK_STREAM) as sock:
        sock.connect((APP_HOST, APP_PORT))
        msg = build_message(msg_code, payload)
        sock.sendall(msg)
        # print(f"Sent {CODE_STR_MAP.get(msg_code, 'UNKNOWN')}: code={hex(msg_code)}, len={len(payload)} payload={payload.hex()}")
        print(print_msg_like_utils(msg))
        response = sock.recv(2048)
        print(f"Received response {len(response)} bytes from app")
  except Exception as e:
    print(f"Send error: {e}")

def send_periodic_requests():
  while True:
    send_request(0x01, make_random_topology())
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
          print(print_msg_like_utils(data))

def print_msg_like_utils(data):
  if len(data) < 8:
    print("Invalid message (too short)")
    return
  res = ''
  msg_code = int.from_bytes(data[0:4], byteorder='big')
  payload_len = int.from_bytes(data[4:8], byteorder='big')
  payload = data[8:8+payload_len]
  code_str = CODE_STR_MAP.get(msg_code, "UNKNOWN")
  res += f"Msg code: {msg_code:08x} ({code_str})\n"
  res += f"Payload len: {payload_len}\n"
  if payload_len:
    res += "Payload data:"
    for i in range(payload_len):
      if i % 16 == 0:
        res += f'\n'
      res += f" 0x{payload[i]:02x}"
    res += f'\n'
  return res

if __name__ == "__main__":
  t1 = threading.Thread(target=send_periodic_requests, daemon=True)
  t2 = threading.Thread(target=tcp_server, daemon=True)
  t1.start()
  t2.start()
  print("Bot started. Press Ctrl+C to exit.")
  try:
    # Set config params
    send_request(0x03, build_settings_payload())
    
    # Get config params
    send_request(0x02)
    
    while True:
      time.sleep(1)
  except KeyboardInterrupt:
    print("Bot stopped.")
  