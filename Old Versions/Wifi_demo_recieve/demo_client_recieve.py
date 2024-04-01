import socket

# IP address and port of the DISCO_L475VG_IOT01A board
HOST = '192.168.193.213'
PORT = 80

print("start")

# Create a TCP/IP socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    try:
        # Connect to the server
        s.connect((HOST, PORT))

        while True:
            # Receive data
            data = s.recv(1024)

            # Check if data is received
            if not data:
                break

            # Print received data
            print('Received:', data.decode())

    except Exception as e:
        print("Error:", e)
