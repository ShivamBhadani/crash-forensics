from hashlib import pbkdf2_hmac
import binascii

password = b"user1234"
salt = bytes([
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0
])
iterations = 1000
dklen = 32

# Perform PBKDF2 with SHA-256
key = pbkdf2_hmac('sha256', password, salt, iterations, dklen)
print("Hashed Password:", binascii.hexlify(key).decode())
