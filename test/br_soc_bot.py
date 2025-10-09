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
    0x04: "RESTART_BR",
    0x05: "STOP_BR"
}

APP_HOST = "::1"
APP_PORT = 11500
BOT_LISTEN_HOST = "::"
BOT_LISTEN_PORT = 11501
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
    addr_template[15] = i + 1
    res.extend(addr_template)
    
    res.extend(parent)
    res.extend([0x00] * 16)
    
    # random parent update
    if  bool(random.getrandbits(1)):
      parent = copy.deepcopy(addr_template)
    
  return bytes(res)

# Build settings payload for SET_CONFIG_PARAMS, matching C struct
def build_settings_payload():
  payload = bytes([
    0x57, 0x69, 0x2d, 0x53, 0x55, 0x4e, 0x20, 0x4e, 
    0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
    0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x66, 0x64, 0x31, 0x32,
    0x3a, 0x33, 0x34, 0x35, 0x36, 0x3a, 0x3a, 0x2f, 
    0x36, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x20, 0x16, 0x2c, 0x01, 0x00, 
    0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x03, 
    0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00
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
  