
Linux Kernel Powerball Module
=============================

A simple Linux Kernel module for getting a Powerball<sub>[0]</sub> Lottery "quick pick".


PREREQUISITES
-------------

Install Linux Kernel headers and source or a custom Linux Kernel.


INSTALL
-------

```bash
make
```

```bash
insmod powerball.ko
```


USAGE
-----

```bash
cat /dev/powerball
```
12 23 34 45 56 17


[0] http://www.powerball.com/