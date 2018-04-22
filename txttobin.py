import sys
data = file(sys.argv[1], 'rb').readlines()
if sys.argv[1].endswith('.txt'):
    uid = sys.argv[1][:-4]

def parity(x):
    return ("{0:08b}".format(x).count('1') & 1)

uid=raw_input("uid> ")
fp = file(uid+'_generated.bin','wb')
fp.write(uid.decode('hex')+"\x00"*2)
bits = 0
pbits = 0
for l in data:
    if bits == 8:
        fp.write(chr(pbits))
        bits = 0
        pbits = 0
    hexbytes = filter(None, l.strip().split(' '))
    bits += 4
    for x in hexbytes:
        b = int(x.replace('!', ''), 16)
        p = parity(b)
        if '!' not in x:
            p^=1
        pbits <<= 1
        pbits |= p
        fp.write(chr(b))

