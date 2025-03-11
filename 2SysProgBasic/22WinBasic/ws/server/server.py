import socket
import struct
import asyncio
import json
import os
import yara
from websockets.asyncio.server import serve
import websockets.exceptions
from http.server import BaseHTTPRequestHandler, HTTPServer

# sc = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# sc.connect(("8.8.8.8", 80))
# host = sc.getsockname()[0]
# sc.close()

host = "192.168.88.137"
# host = "127.0.1.1"
port = 55555

# simple socket
def wstring_form(buffer):
    num_chars = len(buffer) // 2
    chars = struct.unpack('<{}H'.format(num_chars), buffer)
    return ''.join(chr(char) for char in chars)

# with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#     s.bind((host, port))
#     s.listen()
#     print("Server listening on port {}".format(port))
#     while True:
#         client_connection, client_addr = s.accept()
#         with client_connection:
#             print("Connected by client {}:{}".format(client_addr[0], client_addr[1]))
#             while True:
#                 data = client_connection.recv(1024)
#                 if not data:
#                     break
#                 # client_connection.sendall(data)
#                 print("Message from client {}:{}: {}".format(client_addr[0], client_addr[1], wstring_form(data)))
#         print("Client {}:{} disconnected".format(client_addr[0], client_addr[1]))

# http

# class HTTPRequestHandler(BaseHTTPRequestHandler):
#     def do_GET(self):
#         print("GET")
#         self.send_response(200)
#         self.end_headers()
        
# httpd = HTTPServer((host, port), HTTPRequestHandler)
# httpd.serve_forever()

rules = []
for yar_file in os.listdir("./rules"):
    rules.append(yara.compile("./rules/" + yar_file))

def yara_detect(file_path):
    for rule in rules:
        matches = rule.match(file_path)
    return matches

async def handler(websocket):
    print("Connected to client at {}:{}".format(websocket.remote_address[0], websocket.remote_address[1]))
    try:
        async for event in websocket:
            # print(event)
            event_json = json.loads(wstring_form(bytes(event, encoding='utf8')))
            print("{}:{}> {}".format(websocket.remote_address[0], websocket.remote_address[1], event_json))

            log_folder = "./events"
            if not os.path.exists(log_folder):
                os.makedirs(log_folder)
            current_pc_log_folder = log_folder + "/" + event_json["computerName"]
            if not os.path.exists(current_pc_log_folder):
                os.makedirs(current_pc_log_folder)
            current_event_log_file_path = current_pc_log_folder + "/event" + event_json["eventId"] + ".json"
            with open(current_event_log_file_path, "w") as f:
                f.write(json.dumps(event_json))
                
            yara_matches = yara_detect(current_event_log_file_path)
            if yara_matches:
                print("Server> File {} at client {}:{} (computer {}) may contains {}".format(
                    event_json["imageFileDir"],
                    websocket.remote_address[0],
                    websocket.remote_address[1],
                    event_json["computerName"],
                    yara_matches))
    except websockets.exceptions.ConnectionClosedError:
        print("Client at {}:{} disconnected".format(websocket.remote_address[0], websocket.remote_address[1]))

async def main():
    async with serve(handler, host, port, ping_interval=None):
        print("Server is on at {}:{}".format(host, port))
        await asyncio.get_running_loop().create_future()

asyncio.run(main())

