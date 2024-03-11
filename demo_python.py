import socket

# IP address and port of the DISCO_L475VG_IOT01A board
HOST = '192.168.2.52'
PORT = 80
print("start")
# Create a TCP/IP socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    # Connect to the server
    s.connect((HOST, PORT))
    
    # Send data
    s.sendall(b'GET / HTTP/1.1\r\n\r\n')

    
    # Receive response
    data = s.recv(1024)
    
    # Print received data
    print('Received:', data.decode())
