#!/usr/bin/env python3
#
# zpack.py by Jari Komppa
# http://iki.fi/sol/
# based on, and released under same license as:
#
# /* -------------------------------------------------------------------
# zpack - simple LZ77-based data compression
# by Zerkman / Sector One
# ------------------------------------------------------------------- */
#
# /* Copyright © 2020 François Galea <fgalea at free.fr>
# * This program is free software. It comes without any warranty, to
# * the extent permitted by applicable law. You can redistribute it
# * and/or modify it under the terms of the Do What The Fuck You Want
# * To Public License, Version 2, as published by Sam Hocevar. See
# * the COPYING file or http://www.wtfpl.net/ for more details. */
#
#
# This is a pretty direct python conversion, so some things
# could probably be more pythonic.

import sys


def pack(d):
    o = []
    r = 0
    individual_count = 0
    individual = 0

    def count_similar(l1, l2):
        similar = 0
        i = 0
        while i < len(l2):
            if l1[i] != l2[i]:
                return similar
            similar += 1
            i += 1
        return similar

    def flush_individual():
        nonlocal d
        nonlocal individual
        nonlocal individual_count
        o.append((individual_count - 1) | 0xc0)
        for x in range(individual_count):
            o.append(d[individual])
            individual += 1
        individual_count = 0

    while r < len(d):
        p = r - 1
        best_size = 0
        best_pos = p
        while p > 0 and p >= (r - 255):
            size = count_similar(d[p:], d[r:])
            if size > best_size:
                best_size = size
                best_pos = p
            p -= 1
        if best_size > 3:
            # copy
            if individual_count > 0:
                flush_individual()
            if best_size > 0x7f + 0x40 + 4:
                best_size = 0x7f + 0x40 + 4
            individual += best_size
            offset = best_pos - r
            r += best_size
            best_size -= 4
            assert offset >= -256
#           print(f"size={best_size} offset={offset}")
            o.append(best_size)
            if offset < 0:
                offset += 256
            o.append(offset)
        else:
            # individual bytes
            individual_count += 1
            if individual_count == 0x40:
                flush_individual()
            r += 1
    if individual_count > 0:
        flush_individual()
    return bytes(o)


def unpack(d):
    o = []
    size = 0
    out_count = 0
    run = False
    for x in d:
        if out_count:
            o.append(x)
            out_count -= 1
        elif run:
            if x > 127:
                x -= 256
            offset = -256 | x
#           print(f"size={size} offset={offset}")
            size += 3
            while size >= 0:
                o.append(o[len(o) + offset])
                size -= 1
            run = False
        else:
            size = x
            if (size & 0xc0) == 0xc0:
                size &= 0x3f
                out_count = size + 1
            else:
                run = True
    return bytes(o)


def main():
    if len(sys.argv) < 2:
        print("Give me a filename to munch on.")
        return
    in_file = b''
    with open(sys.argv[1], 'rb') as f:
        in_file = f.read()
    out_file = pack(in_file)
    print(f"size={len(in_file)} packed={len(out_file)}\n")
    buffer = unpack(out_file)
    if buffer != in_file:
        print(f"Problem {len(in_file)} {len(buffer)}\n")
        with open("out.upk", "wb") as f:
            f.write(buffer)
    else:
        print("ok")
    with open("out.pck", "wb") as f:
        f.write(out_file)


main()
