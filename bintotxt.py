import sys
with open(sys.argv[1], 'rb') as f:
    data = f.read()

uid = data[0:4]

def parity(x):
    return bin(x).count('1') & 1

for idx in range(6, len(data), 9):
    n = data[idx:idx+8]
    p = data[idx+8]
    nonce_string = [
        "{:02x}".format(n[i]) + ('  ' if int(x) ^ parity(n[i]) else '! ')
        for i, x in enumerate("{:08b}".format(p))
    ]
    print(''.join(nonce_string[:4]))
    print(''.join(nonce_string[4:]))
