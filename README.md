# LZW-Codec

## Overview
### Lempel-Ziv-Welch compression/decompression

This program utilizes Lempel-Ziv-Welch compression/decompression to decrease file size. 

## Usage

The program produces `.lzw` compressed archive files that can be decompressed.

### Compression

`./lzw -c <file>`

### Decompression

`./lzw -d <file>`

## Test Cases

A PPM image (`golfcore.ppm`), text file (`declaration.txt`), and a binary file (`hello`) are included in this repository.
