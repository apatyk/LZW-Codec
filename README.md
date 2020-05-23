# LZW-Codec

## Overview
### Lempel-Ziv-Welch compression/decompression

This program utilizes Lempel-Ziv-Welch compression/decompression to decrease file size. A derivative of this algorithm is used as the default codec for Linux compression, PostScript, and others.

This algorithm is built on the premise of creating a custom dictionary of patterns in a given file that can be used to reduce file size. Thus, this codec works best on files with repeating patterns of data.

## Usage

The program produces `.lzw` compressed archive files that can be decompressed.

### Compression

`./lzw -c <file>`

### Decompression

`./lzw -d <file>`

## Test Cases

A PPM image (`golfcore.ppm`), text file (`declaration.txt`), and a binary file (`hello`) are included in this repository.
