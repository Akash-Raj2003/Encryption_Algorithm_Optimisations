import serial
import time
import threading
import re

#CONFIGURATION
PORT = 'COM7'
BAUDRATE = 115200
TIMEOUT = 0.5

#XTEA DECRYPTION
key = [0xABCDEFAB, 0xEFABCDEF, 0xCDEFABCD, 0xABCDEFAB]

def xtea_decrypt(block, key):
    v0, v1 = block
    delta = 0x9E3779B9
    sum = 0xC6EF3720
    for _ in range(32):
        v1 = (v1 - (((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + key[(sum >> 11) & 3]))) & 0xFFFFFFFF
        sum = (sum - delta) & 0xFFFFFFFF
        v0 = (v0 - (((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + key[sum & 3]))) & 0xFFFFFFFF
    return v0, v1

def decrypt_to_text(blocks):
    b = b''.join(v.to_bytes(4, 'big') for v in blocks)
    try:
        return b.decode('ascii')
    except:
        return b

#Connect to STM32
try:
    ser = serial.Serial(PORT, BAUDRATE, timeout=TIMEOUT)
    time.sleep(2)
except serial.SerialException as e:
    print(f" Failed to open {PORT}: {e}")
    exit(1)

#Reader Thread
def read_from_board():
    while True:
        if ser.in_waiting:
            try:
                line = ser.readline().decode(errors='ignore').strip()
                if not line:
                    continue
                print(f"< {line}")

                #Decrypt Plate
                m = re.match(r"Encrypted Plate: ([0-9a-fA-F]+) ([0-9a-fA-F]+)", line)
                if m:
                    d = xtea_decrypt((int(m[1], 16), int(m[2], 16)), key)
                    print(f"> Decrypted Plate Number: {d[0]}")

                #Decrypt Secret
                m2 = re.match(r"Encrypted secret: ([0-9a-fA-F]+) ([0-9a-fA-F]+) ([0-9a-fA-F]+) ([0-9a-fA-F]+)", line)
                if m2:
                    enc = [int(m2[i], 16) for i in range(1, 5)]
                    part1 = xtea_decrypt((enc[0], enc[1]), key)
                    part2 = xtea_decrypt((enc[2], enc[3]), key)
                    text = decrypt_to_text(part1 + part2)
                    print(f"> Decrypted Secret: {text}")

                #Decrypt UID
                m3 = re.match(r"UID Encrypted: ([0-9a-fA-F]+) ([0-9a-fA-F]+) ([0-9a-fA-F]+) ([0-9a-fA-F]+)", line)
                if m3:
                    enc = [int(m3[i], 16) for i in range(1, 5)]
                    dec1 = xtea_decrypt((enc[0], enc[1]), key)
                    dec2 = xtea_decrypt((enc[2], enc[3]), key)
                    print(f"> Decrypted UID: {dec1[0]:08X} {dec1[1]:08X} {dec2[0]:08X}")

            except Exception as e:
                print(f" Error reading: {e}")

#Launch Reader
threading.Thread(target=read_from_board, daemon=True).start()

#Manual Input Loop
print("📡 Ground Station is ready.")
print("Type commands: start, plate, secret. Press Ctrl+C to exit.")

try:
    while True:
        cmd = input("> ").strip()
        if cmd:
            ser.write((cmd + '\n').encode())
except KeyboardInterrupt:
    print("\n✅ Exiting Ground Station.")
    ser.close()
