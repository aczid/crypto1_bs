all:
	gcc -O3 craptev1-v1.0/solve.c -fPIC -shared -o solve.so
	gcc -O3 -mpopcnt -std=c99 solve_bs.c crypto1_bs.c crypto1_bs_crack.c -Icraptev1-v1.0 craptev1-v1.0/craptev1.c crapto1-v3.3/crapto1.c ./solve.so -o solve_bs -lpthread
	gcc -O3 -mpopcnt -std=c99 solve_piwi_bs.c crypto1_bs.c crypto1_bs_crack.c -Icraptev1-v1.0 craptev1-v1.0/craptev1.c crapto1-v3.3/crapto1.c -o solve_piwi_bs -lpthread
	gcc -O3 -mpopcnt solve_piwi.c -I craptev1-v1.0 craptev1-v1.0/craptev1.c -o solve_piwi -lpthread

clean:
	rm solve.so solve_bs solve_piwi_bs solve_piwi

get_craptev1:
	wget http://crapto1.netgarage.org/craptev1-v1.0.tar.xz
	tar Jxvf craptev1-v1.0.tar.xz

get_crapto1:
	wget http://crapto1.netgarage.org/crapto1-v3.3.tar.xz
	mkdir crapto1-v3.3
	tar Jxvf crapto1-v3.3.tar.xz -C crapto1-v3.3

