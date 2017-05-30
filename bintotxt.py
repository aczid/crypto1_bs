import sys
data = file(sys.argv[1], 'rb').read()
uid = data[0:4]

def parity(x):
    return ("{0:08b}".format(x).count('1') & 1)

for idx in range(6, len(data), 9):
    n = data[idx:idx+8]
    p = data[idx+8]
    nonce_string = [ "%02x" % ord(n[i])
                      + ('  ' if int(x)^parity(ord(n[i])) else '! ')
                      for i, x in enumerate(("{0:08b}".format(ord(p))))
                   ]
    print ''.join(nonce_string[:4])
    print ''.join(nonce_string[4:])
