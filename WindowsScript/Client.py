import subprocess
import time
import socket
import os

# MAC Address and port of the DISCO_L475VG_IOT01A board
BOARD_MAC = "c4-7f-51-8f-76-3c"
LGTVCOMP_PATH = "D:\Tools\LGTV\LGTVcli.exe"
PORT = 80
brightness = [20, 50, 80, 100]
curr_brightness = 0


def turn_off_screen():
    # Command to turn off the screen using LGTVcli.exe
    command = f'{LGTVCOMP_PATH} -screenoff'

    # Retry logic with a maximum of 3 attempts
    max_attempts = 3
    for attempt in range(1, max_attempts + 1):
        try:
            # Run the command and capture the output
            result = subprocess.run(command, capture_output=True, shell=True, text=True)
            # Check if the command was successful (return code 0)
            if result.returncode == 0:
                print("Screen turned off successfully!")
                return True
            else:
                # If the command failed, print the error message and retry
                print(f"Attempt {attempt}: Failed to turn off the screen - {result.stderr.strip()}")
                time.sleep(2)  # Wait for 2 seconds before retrying
        except Exception as e:
            print(f"Attempt {attempt}: Error occurred - {e}")
            time.sleep(2)  # Wait for 2 seconds before retrying

    # All attempts failed
    print(f"All {max_attempts} attempts failed to turn off the screen.")
    return False


def turn_on_screen():
    # Command to turn off the screen using LGTVcli.exe
    command = f'{LGTVCOMP_PATH} -screenon'

    # Retry logic with a maximum of 3 attempts
    max_attempts = 3
    for attempt in range(1, max_attempts + 1):
        try:
            # Run the command and capture the output
            result = subprocess.run(command, capture_output=True, shell=True, text=True)
            # Check if the command was successful (return code 0)
            if result.returncode == 0:
                print("Screen turned on successfully!")
                return True
            else:
                # If the command failed, print the error message and retry
                print(f"Attempt {attempt}: Failed to turn on the screen - {result.stderr.strip()}")
                time.sleep(2)  # Wait for 2 seconds before retrying
        except Exception as e:
            print(f"Attempt {attempt}: Error occurred - {e}")
            time.sleep(2)  # Wait for 2 seconds before retrying

    # All attempts failed
    print(f"All {max_attempts} attempts failed to turn on the screen.")
    return False


def set_hdmi3():
    # Command to turn off the screen using LGTVcli.exe
    command = f'{LGTVCOMP_PATH} -sethdmi3'

    # Retry logic with a maximum of 3 attempts
    max_attempts = 3
    for attempt in range(1, max_attempts + 1):
        try:
            # Run the command and capture the output
            result = subprocess.run(command, capture_output=True, shell=True, text=True)
            # Check if the command was successful (return code 0)
            if result.returncode == 0:
                print("Screen turned on successfully!")
                return True
            else:
                # If the command failed, print the error message and retry
                print(f"Attempt {attempt}: Failed to set hdmi3 - {result.stderr.strip()}")
                time.sleep(2)  # Wait for 2 seconds before retrying
        except Exception as e:
            print(f"Attempt {attempt}: Error occurred - {e}")
            time.sleep(2)  # Wait for 2 seconds before retrying

    # All attempts failed
    print(f"All {max_attempts} attempts failed to set hdmi3.")
    return False


def set_hdmi4():
    # Command to turn off the screen using LGTVcli.exe
    command = f'{LGTVCOMP_PATH} -sethdmi4'

    # Retry logic with a maximum of 3 attempts
    max_attempts = 3
    for attempt in range(1, max_attempts + 1):
        try:
            # Run the command and capture the output
            result = subprocess.run(command, capture_output=True, shell=True, text=True)
            # Check if the command was successful (return code 0)
            if result.returncode == 0:
                print("Screen turned on successfully!")
                return True
            else:
                # If the command failed, print the error message and retry
                print(f"Attempt {attempt}: Failed to set hdmi4 - {result.stderr.strip()}")
                time.sleep(2)  # Wait for 2 seconds before retrying
        except Exception as e:
            print(f"Attempt {attempt}: Error occurred - {e}")
            time.sleep(2)  # Wait for 2 seconds before retrying

    # All attempts failed
    print(f"All {max_attempts} attempts failed to set hdmi4.")
    return False


def toggle_brightness():
    # Command to toggle brightness using LGTVcli.exe
    global curr_brightness
    curr_brightness = (curr_brightness + 1) % len(brightness)
    command = f'{LGTVCOMP_PATH} -backlight {brightness[curr_brightness]}'

    # Retry logic with a maximum of 3 attempts
    max_attempts = 3
    for attempt in range(1, max_attempts + 1):
        try:
            # Run the command and capture the output
            result = subprocess.run(command, capture_output=True, shell=True, text=True)
            # Check if the command was successful (return code 0)
            if result.returncode == 0:
                print(f"Brightness changed to {brightness[curr_brightness]}")
                return True
            else:
                # If the command failed, print the error message and retry
                print(f"Attempt {attempt}: Failed to toggle brightness - {result.stderr.strip()}")
                time.sleep(2)  # Wait for 2 seconds before retrying
        except Exception as e:
            print(f"Attempt {attempt}: Error occurred - {e}")
            time.sleep(2)  # Wait for 2 seconds before retrying

    # All attempts failed
    print(f"All {max_attempts} attempts failed to set hdmi4.")
    return False


# Create a TCP/IP socket
if __name__ == '__main__':
    cmd = f'arp -a | findstr "{BOARD_MAC}" '
    # Windows arp table only store directly interacted ips
    got_ip = False
    while not got_ip:
        try:
            returned_output = subprocess.check_output((cmd), shell=True, stderr=subprocess.STDOUT)
            parse = str(returned_output).split(' ', 1)
            ip = parse[1].split(' ')[1]
            print(f"{BOARD_MAC} found, ip: {ip}.")
            got_ip = True
        # Board not found, Ping all ip in local net to find the board
        except subprocess.CalledProcessError:
            print(f"{BOARD_MAC} not found, start Pinging.")
            for i in range(256):
                ip = f'10.0.0.{i}'
                response = os.system(f"ping  -n 1 -w 200 {ip}")
                if response == 0:  # if ping is successful
                    try:
                        # Check if board is found
                        returned_output = subprocess.check_output((cmd), shell=True, stderr=subprocess.STDOUT)
                        parse = str(returned_output).split(' ', 1)
                        ip = parse[1].split(' ')[1]
                        print(f"{BOARD_MAC} found, ip: {ip}.")
                        got_ip = True
                        break
                    except subprocess.CalledProcessError:
                        continue

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            # Connect to the server
            s.connect((ip, PORT))

            while True:
                # Receive data
                data = s.recv(1024)

                # Check if data is received
                if data and data.decode() == "cmd1":
                    turn_off_screen()
                    print('Received:', data.decode())
                elif data and data.decode() == "cmd2":
                    turn_on_screen()
                    print('Received:', data.decode())
                elif data and data.decode() == "cmd3":
                    set_hdmi3()
                    print('Received:', data.decode())
                elif data and data.decode() == "cmd4":
                    set_hdmi4()
                    print('Received:', data.decode())
                elif data and data.decode() == "cmd5":
                    toggle_brightness()
                    print('Received:', data.decode())
                elif data:
                    print('Received:', data.decode())

        except Exception as e:
            print("Error:", e)