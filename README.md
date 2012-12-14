On my Intel i7-2620M quadcore:

    $ ./speedtest-copied
    1 threads: 1999113 hashes/s, total = 5.002s
    2 threads: 3443722 hashes/s, total = 2.904s
    4 threads: 3709510 hashes/s, total = 2.696s
    8 threads: 3665865 hashes/s, total = 2.728s
    12 threads: 3650451 hashes/s, total = 2.739s
    24 threads: 3642619 hashes/s, total = 2.745s

    $ ./speedtest-locked
    1 threads: 2013590 hashes/s, total = 4.966s
    2 threads: 857542 hashes/s, total = 11.661s
    4 threads: 631336 hashes/s, total = 15.839s
    8 threads: 932238 hashes/s, total = 10.727s
    12 threads: 850431 hashes/s, total = 11.759s
    24 threads: 802501 hashes/s, total = 12.461s

And on an Intel Xeon X5650 24 core machine:

    $ ./speedtest-copied
    1 threads: 1564546 hashes/s, total = 6.392s
    2 threads: 1973912 hashes/s, total = 5.066s
    4 threads: 3821067 hashes/s, total = 2.617s
    8 threads: 5096136 hashes/s, total = 1.962s
    12 threads: 5849133 hashes/s, total = 1.710s
    24 threads: 7467990 hashes/s, total = 1.339s

    $ ./speedtest-locked
    1 threads: 1481025 hashes/s, total = 6.752s
    2 threads: 701797 hashes/s, total = 14.249s
    4 threads: 338231 hashes/s, total = 29.566s
    8 threads: 318873 hashes/s, total = 31.360s
    12 threads: 402054 hashes/s, total = 24.872s
    24 threads: 304193 hashes/s, total = 32.874s

More implementation details can be found in
[this blog post](http://blog.plenz.com/2012-12/concurrent-hashing-is-an-embarrassingly-parallel-problem.html).
