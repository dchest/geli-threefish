# geli-threefish

![Logo](logo.png)

**geli-threefish** is a set of patches for FreeBSD
[geom-eli](http://www.freebsd.org/cgi/man.cgi?query=geli) and
[crypto](http://www.frecrypto](ebsd.org/cgi/man.cgi?query=crypto&apropos=0&sektion=4)
kernel modules to support disk encryption with
[Threefish-512](http://en.wikipedia.org/wiki/Threefish) algorithm.

*This software is experimental. Use at your own risk! Tested on FreeBSD 9.1.*

## Why?

I wanted to install FreeBSD into the encrypted disk on my Atom-based netbook,
but didn't want to suffer from the slowness of AES. Since the CPU doesn't have
AES-NI instructions, AES-128 CBC from OpenSSL runs at about 26 MB/s (AES-XTS
should be even slower), while this Threefish implementation achieves about 150
MB/s.

Basically, **geli-threefish** makes your encrypted disk ~3x faster
without sacrificing security.


## Why not?

You don't want to use these patches if:

  *  you have a modern processor with
     [AES-NI instructions](http://en.wikipedia.org/wiki/AES_instruction_set)
     (AES will be faster than Threefish on it), or

  *  you don't want to manage custom kernel modules, or

  *  you trust AES more than Threefish.


## How to install

See (Installation instructions)[INSTALL.md].

or, "I know what I'm doing, [just gimme the patch](https://github.com/dchest/geli-threefish/compare/freebsd...master.patch)"!


## Technical details

  *  Threefish-512 is used in "tweak counter" mode. The first 64-bit part of
     tweak is a sector number, the last 64-bit part of tweak is a block counter.
     The block counter is incremented for each 512-bit block of the sector.

  *  Encryption and decryption operations of Threefish are swapped, as
     recommended by the Skein paper to achieve better read performance,
     which is the more common operation. This doesn't affect security of the
     cipher.

  *  Metadata (master keys, etc.) is encrypted with AES-256 in CBC mode. This
     simplifies code and doesn't affect performance.

## License

Threefish implementation is licensed under the FreeBSD license,
modifications to `geom-eli` and `crypto` are placed into the public domain.
