/* -------------------------------------------------------------------
 zpack - simple LZ77-based data compression
 by Zerkman / Sector One
------------------------------------------------------------------- */

/* Copyright © 2020 François Galea <fgalea at free.fr>
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * the COPYING file or http://www.wtfpl.net/ for more details. */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>

static int count_similar(const unsigned char *p1, const unsigned char *p2,
                         const unsigned char *end) {
  int similar = 0;
  while (p2 < end && *p1 == *p2) {
    ++similar;
    p1 ++;
    p2 ++;
  }
  return similar;
}

#define flush_individual { \
  *w++ = (individual_count-1) | 0xc0; \
  memcpy(w, individual, individual_count); \
  w += individual_count; \
  individual += individual_count; \
  individual_count = 0; }

long pack(unsigned char *out, const unsigned char *in, long size) {
  unsigned char *w = out;
  const unsigned char *r = in;
  const unsigned char *end = in + size;
  const unsigned char *individual = in;
  int individual_count = 0;
  while (r < end) {
    const unsigned char *p = r-256;
    int best_size = 3;
    const unsigned char *best_pos = p;
    if (p < in)
        p = in;
    while (p < r - 1 && best_size < 0x7f + 0x40 + 4) {
      int size = count_similar(p, r, end);
      if (size > best_size) {
        best_size = size;
        best_pos = p;
      }
      p ++;
    }

    if (best_size > 3) {
      /* copy */
      if (individual_count)
        flush_individual;
      if (best_size > 0x7f+0x40+4)
        best_size = 0x7f+0x40+4;
      individual += best_size;
      int offset = best_pos - r;
      r += best_size;
      best_size -= 4;

      assert(offset >= -256);
      *w++ = best_size;
      *w++ = offset;
      /* printf("size=%d offset=%d\n", best_size, offset); */
    } else {
      /* individual bytes */
      individual_count ++;
      if (individual_count == 0x40)
        flush_individual;
      r ++;
    }
  }
  if (individual_count)
    flush_individual;
  return w-out;
}

long unpack(unsigned char *out, const unsigned char *in, long size) {
  unsigned char *w = out;
  const unsigned char *r = in;
  const unsigned char *end = in + size;
  while (r < end) {
    /* printf("%4x %4x\n", w-out, r-in); */
    int size = (unsigned char)(*r++);
    if ((size & 0xc0) == 0xc0) {
      size &= 0x3f;
      while (size-- >= 0)
        *w ++ = *r ++;
    } else {
      int offset;
      offset = -256 | (signed char)(*r++);
      /* printf("size=%d offset=%d\n", size, offset); */
      size += 3;
      while (size-- >= 0) {
        *w = *(w+offset);
        w ++;
      }
    }
  }
  return w-out;
}

int main(int argc, char **argv) {
  FILE *fd = fopen(argv[1], "rb");
  if (!fd) {
    perror(argv[1]);
    return 1;
  }

  fseek(fd, 0, SEEK_END);
  long in_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  unsigned char *in_file = (unsigned char*)malloc(in_size);
  unsigned char *out_file = (unsigned char*)malloc(in_size*2);
  fread(in_file, 1, in_size, fd);
  fclose(fd);

  long packed_size = pack(out_file, in_file, in_size);

  printf("size=%ld packed=%ld\n", in_size, packed_size);

  unsigned char *buffer = (unsigned char*)malloc(in_size*2);
  long unpacked_size = unpack(buffer, out_file, packed_size);
  if (memcmp(in_file, buffer, in_size)) {
    printf("Problem %ld %ld\n", in_size, unpacked_size);
    fd = fopen("out.upk", "wb");
    fwrite(buffer, 1, unpacked_size, fd);
    fclose(fd);
  }
  else
    printf("Depack ok\n");

  fd = fopen("out.pck", "wb");
  fwrite(out_file, 1, packed_size, fd);
  fclose(fd);
  free(buffer);
  free(out_file);
  free(in_file);

  return 0;
}
