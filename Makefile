# if -march=native does not work on your platform, you could try
# -msse
# -msse2
# -mavx
# or
# -mavx2

CC     = gcc
CFLAGS = -std=gnu99 -O3 -march=native
# popcount is used for fast (hardware) parity computation on non-bitsliced states

all: solve_bs solve_piwi_bs solve_piwi libnfc_crypto1_crack

CRAPTEV1_CODE = craptev1-v1.0/craptev1.c -I craptev1-v1.0/
CRAPTO1_CODE = crapto1-v3.3/crapto1.c crapto1-v3.3/crypto1.c -I crapto1-v3.3/ 
CRYPTO1_BS_CODE = crypto1_bs.c crypto1_bs_crack.c 

solve.so:
	$(CC) $(CFLAGS) craptev1-v1.0/solve.c -fPIC -shared -o solve.so

solve_bs: solve.so
	$(CC) $(CFLAGS) $@.c $(CRYPTO1_BS_CODE) $(CRAPTO1_CODE) ${CRAPTEV1_CODE} ./solve.so -o $@ -lpthread

solve_piwi_bs:
	$(CC) $(CFLAGS) $@.c $(CRYPTO1_BS_CODE) $(CRAPTO1_CODE) ${CRAPTEV1_CODE} -o $@ -lpthread

solve_piwi:
	$(CC) $(CFLAGS) $@.c $(CRYPTO1_BS_CODE) $(CRAPTO1_CODE) ${CRAPTEV1_CODE} -o $@ -lpthread

libnfc_crypto1_crack:
	$(CC) $(CFLAGS) $@.c $(CRYPTO1_BS_CODE) $(CRAPTO1_CODE) ${CRAPTEV1_CODE} -o $@ -lpthread -lnfc -lm

clean:
	rm -f solve.so solve_bs solve_piwi_bs solve_piwi libnfc_crypto1_crack

get_craptev1:
	wget http://crapto1.netgarage.org/craptev1-v1.0.tar.xz
	tar Jxvf craptev1-v1.0.tar.xz

get_crapto1:
	wget http://crapto1.netgarage.org/crapto1-v3.3.tar.xz
	mkdir crapto1-v3.3
	tar Jxvf crapto1-v3.3.tar.xz -C crapto1-v3.3
