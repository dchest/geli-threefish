# geli-threefish Installation

0. You should have FreeBSD source code installed.
See [Syncronizing Your Source](http://www.freebsd.org/doc/en_US.ISO8859-1/books/handbook/synching.html)
in the FreeBSD Handbook. We'll assume sources are located in `/usr/src/`.

1. Download **geli-threefish** sources. The easiest way to do is to go to the
https://github.com/dchest/geli-threefish and click "Download ZIP".

2. Replace files in:

* `/usr/src/sys/geom/eli/`
* `/usr/src/sys/opencrypto/`
* `/usr/src/sys/modules/crypto/`

with the ones from **geli-threefish**.

3. Rebuild and install `crypto` kernel module (you may need to set
`MAKESYSPATH` environment variable to `/usr/src/share/mk` before building):

    # cd /usr/src/modules/crypto
    # make install

4. Rebuild and install `geom` kernel module:

    # cd /usr/src/modules/geom
    # make install

5. Rebuild and install `geli` utility:

    # cd /usr/src/sbin/geli
    # make install

6. If you have `geom_eli` or `crypto` modules loaded, unload them:

    # kldunload geom_eli
    # kldunload crypto

If you have encrypted disks mounted, you'll have to unmount and detach them first.

7. Load updated modules:

    # kldload geom_eli

(This will automatically load `crypto`).

8. Now you will be able to use `geli` with `-e threefish` option.
See [Disk Encryption with geli](http://www.freebsd.org/doc/en_US.ISO8859-1/books/handbook/disks-encrypting.html)
in the FreeBSD Handbook for further information.

For example, to create a new encrypted disk (warning: this is destructive operation):

    # geli init -e threefish -s 4096 /dev/da2

Enter new password and repeat it. Then attach the device:

    # geli attach /dev/da2

Enter your password. Now you can access the disk `/dev/da2.eli`.

Create a filesystem on it:

    # newfs /dev/da2.eli

and mount it.

    # mount /dev/da2.eli /private

