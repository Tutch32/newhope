# NewHope

This is a fork of [NewHope by Alkim, Ducas, Pöppelmann and Schwabe](https://github.com/tpoeppelmann/newhope) with some optimizations to the coefficient-sampling method. Details on NewHope are available [here](https://cryptojedi.org/crypto/#newhope) and in the [original paper](https://eprint.iacr.org/2015/1092).

We introduce the following optimizations, that can be followed in the different commits and branches of this repository:
* Decrease the rejection-rate in the coefficient-sampling
* Vectorize the coefficient-sampling using AVX2 and AVX512 instructions
* Exchange SHAKE-128 for pseudorandomness generation with faster SHA-256 or AES-256

The goal is to demonstrate possible optimizations by leveraging modern processor architecture features. The following table presents our performance improvements, measured with the included testbench on an Intel(R) Core(TM) i7-4600U CPU @ 2.70 GHz. The different optimizations are incremental (include the previous ones) except for the last two; there is either SHA-256 or AES-256 for generating pseudorandom bytes.

| Optimization             | Server cycles (keygen+shareda) | Improvement | Client cycles (sharedb) | Improvement |
| ------------------------ | ------------------------------:| -----------:| -----------------------:| -----------:|
| Baseline                 |                         130994 |             |                  138410 |             |
| Decreased rejection-rate |                         120150 |       1.09x |                  126602 |       1.09x |
| AVX2 sampling            |                         109388 |       1.20x |                  120708 |       1.15x |
| SHA-256 generation       |                         100300 |       1.31x |                  106700 |       1.30x |
| AES-256 generation       |                          85316 |       1.54x |                   94116 |       1.47x |

Authors:
--------

* Shay Gueron (1, 2)
* Fabian Schlieker (3)

(1) Intel Corporation, Israel Development Center, Haifa, Israel  
(2) University of Haifa, Israel  
(3) Ruhr University Bochum, Germany