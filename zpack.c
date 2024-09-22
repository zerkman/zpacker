/* -------------------------------------------------------------------
 zpack - simple LZ77-based data compression
 by Zerkman / Sector One
------------------------------------------------------------------- */

/* Copyright © 2020-2023 François Galea <fgalea at free.fr>
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * the COPYING file or http://www.wtfpl.net/ for more details. */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

static int count_similar(const uint8_t *p1, const uint8_t *p2,
                         const uint8_t *end) {
  int similar = 0;
  while (p2 < end && *p1 == *p2) {
    ++similar;
    p1 ++;
    p2 ++;
  }
  return similar;
}

static uint8_t * encode_copy(uint8_t * out, const uint8_t * inp,  int n)
{
  assert( n >= 1 );
  assert( n <= 64 );
  *out++ = n + 191;
  assert( (out[-1] & 0xc0) == 0xc0 );
  memcpy(out,inp,n);
  return out += n;
}

#define flush_individual {			    \
  w = encode_copy(w, individual, individual_count); \
  individual += individual_count;		    \
  individual_count = 0; }

static uint8_t * encode_back(uint8_t * out, int offset, int n)
{
  assert( offset <= -1 );
  assert( offset >= -256 );
  assert( n >= 4 );
  assert( n <= 195 );
  *out++ = 195 - n;
  *out++ = offset;
  return out;
}

long pack(uint8_t *out, const uint8_t *in, long size) {
  uint8_t *w = out;
  const uint8_t *r = in;
  const uint8_t *end = in + size;
  const uint8_t *individual = in;
  int individual_count = 0;
  while (r < end) {
    const uint8_t *p = r-1;
    int best_size = 0;
    const uint8_t *best_pos = p;
    while (p > in && p >= (r-255)) {
      int size = count_similar(p, r, end);
      if (size > best_size) {
        best_size = size;
        best_pos = p;
      }
      p --;
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
      w = encode_back(w, offset, best_size);

      /* best_size -= 4; */
      /* w = encode_back(w, offset, best_size+4); */
      /* *w++ = best_size; */
      /* *w++ = offset; */
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

long unpack(uint8_t *out, const uint8_t *in, long size) {
  uint8_t *w = out;
  const uint8_t *r = in;
  const uint8_t *end = in + size;
  while (r < end) {
    /* printf("%4x %4x\n", w-out, r-in); */
    int size = (uint8_t)(*r++);

    
    if ((size & 0xc0) == 0xc0) {
      size &= 0x3f;
      while (size-- >= 0)
        *w ++ = *r ++;
    } else {
      int offset;
      offset = -256 | (signed char)(*r++);
      /* printf("size=%d offset=%d\n", size, offset); */
      size = 195 - size;
      assert( size >= 4 );
      assert( size <= 195 );
      do {
        *w = w[offset];
        ++w;
      } while (--size);
    }
  }
  return w-out;
}

int main(int argc, char **argv) {
  FILE *fd;
  if (argc < 2 || !strcmp(argv[1],"-h") || !strcmp(argv[1],"--help")) {
    puts("Usage: zpack <INPUT> [<OUTPUT>]");
    return 0;
  }
  
  fd = fopen(argv[1], "rb");
  if (!fd) {
    perror(argv[1]);
    return 1;
  }

  fseek(fd, 0, SEEK_END);
  long in_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  uint8_t in_file[in_size];
  uint8_t out_file[in_size*2];
  fread(in_file, 1, in_size, fd);
  fclose(fd);

  long packed_size = pack(out_file, in_file, in_size);

  printf("%s: size=%ld packed=%ld %ld%%\n", argv[1],
	 in_size, packed_size, packed_size*100/in_size);

  uint8_t buffer[in_size*2];
  long unpacked_size = unpack(buffer, out_file, packed_size);
  if (memcmp(in_file, buffer, in_size)) {
    printf("Problem %ld %ld\n", in_size, unpacked_size);
    fd = fopen("out.upk", "wb");
    fwrite(buffer, 1, unpacked_size, fd);
    fclose(fd);
  }
  else
    printf("Depack ok\n");

  fd = fopen( argc > 2 ? argv[2] : "out.pck" , "wb");
  fwrite(out_file, 1, packed_size, fd);
  fclose(fd);

  return 0;
}
