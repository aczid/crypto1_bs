Bitsliced Crypto-1 brute-forcer
===============================

A pure C(99) implementation of the [Crypto-1 cipher](https://en.wikipedia.org/wiki/Crypto-1) using the method of [bitslicing](https://en.wikipedia.org/wiki/Bit_slicing), which uses GNU vector extensions to be portable across SSE/AVX/AVX2 supporting architectures while offering the highest amount of possible parallelism.


Background
----------

I wrote this as a patch for [piwi's imlementation](https://github.com/pwpiwi/proxmark3/tree/hard_nested/) of the research documented in [Ciphertext-only cryptanalysis on hardened Mifare Classic cards](http://www.cs.ru.nl/~rverdult/Ciphertext-only_Cryptanalysis_on_Hardened_Mifare_Classic_Cards-CCS_2015.pdf) after reading (most of) the paper, while it was still under [active development](http://www.proxmark.org/forum/viewtopic.php?id=2120).

After my development of a somewhat naive brute-forcer, another implementation of the same attack surfaced, [CraptEV1](http://crapto1.netgarage.org/).
With all of its clever tricks that code pushed me to improve my own, eventually reaching a significant (8-10x) speedup.
Besides that, its publication as a library also inspired the included libnfc application that demonstrates the CraptEV1 attack code + my bitsliced cracker.

The proxmark3 patch, as well as the code for piwi's branch that it applies to, have since been merged into [Iceman1001's fork](https://github.com/iceman1001/proxmark3/) along with many other experimental improvements to the proxmark3 codebase.
Much more work has been done in the project since my patch, so I would advise proxmark3 users to use that fork to test the code.
My original patch is included for completeness as `pwpiwi_proxmark3_hard_nested.patch`.

Even later still, this code actually got merged into the upstream [Proxmark3 codebase](https://github.com/Proxmark/proxmark3)! More importantly, it was included as part of an entirely new variation on the attack by [@pwpiwi](https://github.com/pwpiwi/), which requires fewer nonces and achieves a better reduction of potential states, while also improving the brute-forcing phase, ending up as another order of magnitude faster in practice.
My ideal is to also merge his great new tricks into the `libnfc_crypto1_crack` tool. It seems easy to port but I haven't made the effort yet.

Tools
-----

~~The following tools are only available for / tested on 64-bit Linux.~~
Support for 32-bit and 64-bit machines was tested on various Intel/ARM based Linux platforms.
OSX compatibility issues were resolved by [@unkernet](https://github.com/unkernet/).

If you want to use the following stand-alone binaries, you will need the original CraptEV1 / Crapto1 source packages.
For convenience, and because redistribution of CraptEV1 is not allowed, I've added make targets `get_craptev1` and `get_crapto1` to fetch and extract these packages to the current working directory.
I have included a conversion of the test file `0xcafec0de.txt` included in the CraptEV1 package to the binary format used by the `proxmark3/hard_nested` branch.

`solve_bs` is analogous to CraptEV1 `solve` and works on .txt files using the bitsliced crypto-1 cracker

    $ ./solve_bs craptev1-v1.1/0xcafec0de.txt 0xcafec0de

`solve_piwi` uses CraptEV1 on .bin files as gathered by piwi's PM3 code

    $ ./solve_piwi 0xcafec0de.bin

`solve_piwi_bs` does the same but uses the bitsliced cracker

    $ ./solve_piwi_bs 0xcafec0de.bin

`libnfc_crypto1_crack` uses libnfc to demonstrate the CraptEV1 code using the bitsliced cracker

    $ ./libnfc_crypto1_crack 000000000000 0 A 4 A


Acknowledgements
----------------

Special thanks to Carlo Meijer, Roel Verdult, piwi and bla.


License
-------

All the code in this repository is made available under the MIT license, except for the files `pwpiwi_proxmark3_hard_nested.patch` and `libnfc_crypto1_crack.c`, which are GPLv2 due to deriving from respectively Proxmark3 and MFOC.

Copyright (c) 2015-2016 Aram Verstegen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

