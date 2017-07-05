<div style="text-align: center">
<img src="https://github.com/mvs-org/metaverse/raw/master/doc/image/logo.png"/>
</div>
Metaverse Wallet
=============

Setup
---------------------
Metaverse Wallet is the original Metaverse client and it builds the backbone of the network. It downloads and, by default, stores the entire history of Metaverse transactions (which is currently more than 3 GBs); depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to a day or more.

To download Metaverse, visit [mvs.org](https://mvs.org#download/).

Running
---------------------
The following are some helpful notes on how to run Metaverse on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/bitcoin-qt` (GUI) or
- `bin/bitcoind` (headless)

### Windows

Unpack the files into a directory, and then run bitcoin-qt.exe.

### OS X

Drag Metaverse-Core to your applications folder, and then run Metaverse-Core.

### Need Help?

* See the documentation at the [Wikipedia Metaverse](https://en.wikipedia.org/wiki/Metaverse_Blockchain)

Building
---------------------
The following are developer notes on how to build Metaverse on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [OS X Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [OpenBSD Build Notes](build-openbsd.md)

Development
---------------------
The Metaverse repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [Travis CI](travis-ci.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)
