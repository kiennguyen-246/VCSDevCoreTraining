import socket
import struct
import asyncio
from websockets.asyncio.server import serve
from http.server import BaseHTTPRequestHandler, HTTPServer

# sc = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# sc.connect(("8.8.8.8", 80))
# host = sc.getsockname()[0]
# sc.close()

host = "192.168.150.129"
# host = "127.0.1.1"
port = 8000

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

async def handler(websocket):
    print("Connected to client at {}:{}".format(websocket.remote_address[0], websocket.remote_address[1]))
    async for msg in websocket:
        print("{}:{}> {}".format(websocket.remote_address[0], websocket.remote_address[1], msg))
        await websocket.send(msg)
    # await websocket.send(data)

async def main():
    async with serve(handler, host, port, ping_interval=None):
        print("Server is on at {}:{}".format(host, port))
        await asyncio.get_running_loop().create_future()

asyncio.run(main())