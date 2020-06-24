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
import time


def pack(d):
    o = []
    r = 0
    individual_count = 0
    individual = 0
       
    def flush_individual():
        nonlocal d
        nonlocal individual
        nonlocal individual_count
        if individual_count == 0:
            return
        o.append((individual_count - 1) | 0xc0)
        o.extend(d[individual:individual + individual_count])
        individual += individual_count
        individual_count = 0

    while r < len(d):
        p = max(r - 256, 0)
        firstpos = d[p:].find(d[r:r + 4])
        p += firstpos
        best_size = 3
        best_pos = 0
        
        if firstpos != -1:
            while p < r:
                size = 3
                while d[p + size] == d[r + size]:
                    size += 1
                if size > best_size:
                    best_size = size
                    best_pos = p
                    if best_size >= 0x7f + 0x40 + 4:
                        break
                nextpos = d[p + 1:].find(d[r:r + 4]) + 1
                if nextpos == 0:
                    break
                p += nextpos

        if best_size > 3:
            # copy
            flush_individual()
            best_size = min(best_size, 0x7f + 0x40 + 4)
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
    starttime = time.time()
    out_file = pack(in_file)
    endtime = time.time()
    print(f"size={len(in_file)} packed={len(out_file)} time={(endtime-starttime):.2f}")
    buffer = unpack(out_file)
    if buffer != in_file:
        print(f"Problem {len(in_file)} {len(buffer)}")
        with open("out.upk", "wb") as f:
            f.write(buffer)
    else:
        print("ok")
    with open("out.pck", "wb") as f:
        f.write(out_file)


main()
