import sys
with open(sys.argv[1], 'rb') as f:
    data = f.readlines()
if sys.argv[1].endswith('.txt'):
    uid = sys.argv[1][:-4]
def parity(x):
    return ("{0:08b}".format(x).count('1') & 1)

uid = input("uid> ")
uid_bytes = bytes.fromhex(uid)

with open(uid + '_generated.bin', 'wb') as fp:
    fp.write(uid_bytes + b"\x00" * 2)
    bits = 0
    pbits = 0
    for l in data:
        if bits == 8:
            fp.write(bytes([pbits]))
            bits = 0
            pbits = 0
        if isinstance(l, bytes):
            l = l.decode('utf-8')
        hexbytes = [x for x in l.strip().split(' ') if x]
        
        bits += 4
        for x in hexbytes:
            b = int(x.replace('!', ''), 16)
            p = parity(b)
            if '!' not in x:
                p ^= 1
            pbits <<= 1
            pbits |= p
            fp.write(bytes([b]))