Bitsliced Crypto-1 brute-forcer
===============================

A pure C(99) implementation of the [Crypto-1 cipher](https://en.wikipedia.org/wiki/Crypto-1) using the method of [bitslicing](https://en.wikipedia.org/wiki/Bit_slicing), which uses GNU vector extensions to be portable across SSE/AVX/AVX2 supporting architectures while offering the highest amount of possible parallelism.


Background
----------

I wrote this as a patch for [piwi's imlementation](https://github.com/pwpiwi/proxmark3/tree/hard_nested/) of the research documented in [Ciphertext-only cryptanalysis on hardened Mifare Classic cards](http://www.cs.ru.nl/~rverdult/Ciphertext-only_Cryptanalysis_on_Hardened_Mifare_Classic_Cards-CCS_2015.pdf) after reading (most of) the paper, while it was still under [active development](http://www.proxmark.org/forum/viewtopic.php?id=2120).
The final patch is included as `pwpiwi_proxmark3_hard_nested.patch`.

Later on, another implementation of the same attack surfaced, [CraptEV1](http://crapto1.netgarage.org/).
I managed to gather some great tricks from that code, which unfortunately is off-line now (and has a license forbidding redistribution).
This also allowed me to compare my Crypto-1 implementation to a finished brute-forcer, and eventually I managed to significantly beat CraptEV1's (great) performance.

Tools
-----

If you want to use the following stand-alone binaries, you will need the original CraptEV1 / Crapto1 source packages.
For convenience, and because redistribution of CraptEV1 is not allowed, I've added make targets `get_craptev1` and `get_crapto1` to fetch and extract these packages to the current working directory.
I have included a conversion of the test file `0xcafec0de.txt` included in the CraptEV1 package to the binary format used by the `proxmark3/hard_nested` branch.

`solve_bs` is analogous to CraptEV1 `solve` and works on .txt files using the bitsliced crypto-1 cracker

    $ ./solve_bs craptev1-v1.0/0xcafec0de.txt 0xcafec0de

`solve_piwi` uses CraptEV1 on .bin files as gathered by piwi's PM3 code

    $ ./solve_piwi 0xcafec0de.bin

`solve_piwi_bs` does the same but uses the bitsliced cracker

    $ ./solve_piwi_bs 0xcafec0de.bin


Acknowledgements
----------------

Special thanks to Carlo Meijer, Roel Verdult, piwi and bla.

