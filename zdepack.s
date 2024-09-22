;---------------------------------------------------------------------
;	zdepack
;	by zerkman / Sector One
;	size optimisation by Ben / OVR
;---------------------------------------------------------------------

; Copyright (c) 2020-2023 Francois Galea <fgalea at free.fr>
; This program is free software. It comes without any warranty, to
; the extent permitted by applicable law. You can redistribute it
; and/or modify it under the terms of the Do What The Fuck You Want
; To Public License, Version 2, as published by Sam Hocevar. See
; the COPYING file or http://www.wtfpl.net/ for more details.

; unpack data chunk
; parameters:
; a0: beginning of packed data
; a1: unpack buffer
; a2: end of packed data
; returned value:
; a1: end of unpacked data

zdepack:
	moveq	#-1,d1
next:
	moveq.l	#191,d0
	sub.b	(a0)+,d0
	bcc.s	offset

	;; d0 => -1 .. -64
raw:
rawlp:	move.b	(a0)+,(a1)+
	addq.b	#1,d0
	bne.s	rawlp
	bra.s	test

	;; d0 => 0..191
offset:
	move.b	(a0)+,d1
	addq.b	#3,d0

offlp:	move.b	(a1,d1.w),(a1)+
	subq.b	#1,d0
	bcc.s	offlp

test:	cmp.l	a2,a0
	bne.s	next
