

with open('/dev/hidraw0', 'rb+') as hidg0:
    hidg0.write(b'\x00\x00\x04\x00\x00\x00\x00\x00')  # нажали А
    hidg0.write(b'\x00\x00\x00\x00\x00\x00\x00\x00')  # отпустили А