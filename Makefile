# if -march=native does not work on your platform, you could try
# -msse
# -msse2
# -mavx
# or
# -mavx2

CRYPTO1_BS_OPTIMIZATION_ARGS=-O3 -march=native -mpopcnt
# popcount is used for fast (hardware) parity computation on non-bitsliced states

all:
	gcc -O3 craptev1-v1.0/solve.c -fPIC -shared -o solve.so
	gcc -std=c99 ${CRYPTO1_BS_OPTIMIZATION_ARGS} solve_bs.c crypto1_bs.c crypto1_bs_crack.c -Icraptev1-v1.0 craptev1-v1.0/craptev1.c crapto1-v3.3/crapto1.c ./solve.so -o solve_bs -lpthread
	gcc -std=c99 ${CRYPTO1_BS_OPTIMIZATION_ARGS} solve_piwi_bs.c crypto1_bs.c crypto1_bs_crack.c -Icraptev1-v1.0 craptev1-v1.0/craptev1.c crapto1-v3.3/crapto1.c -o solve_piwi_bs -lpthread
	gcc ${CRYPTO1_BS_OPTIMIZATION_ARGS} solve_piwi.c -I craptev1-v1.0 craptev1-v1.0/craptev1.c -o solve_piwi -lpthread
	gcc -std=c99 ${CRYPTO1_BS_OPTIMIZATION_ARGS} -Icraptev1-v1.0 -Icrapto1-v3.3 libnfc_crypto1_crack.c crypto1_bs.c crypto1_bs_crack.c crapto1-v3.3/crypto1.c crapto1-v3.3/crapto1.c craptev1-v1.0/craptev1.c -o libnfc_crypto1_crack -lpthread -lnfc

clean:
	rm solve.so solve_bs solve_piwi_bs solve_piwi

get_craptev1:
	wget http://crapto1.netgarage.org/craptev1-v1.0.tar.xz
	tar Jxvf craptev1-v1.0.tar.xz

get_crapto1:
	wget http://crapto1.netgarage.org/crapto1-v3.3.tar.xz
	mkdir crapto1-v3.3
	tar Jxvf crapto1-v3.3.tar.xz -C crapto1-v3.3

