;---------------------------------------------------------------------
;	zdepack
;	by zerkman / Sector One
;---------------------------------------------------------------------

; unpack data chunk
; parameters:
; a0: beginning of packed data
; a1: unpack buffer
; a2: end of packed data
; returned value:
; a1: end of unpacked data
zdepack:
next:
	move.b	(a0)+,d0
	ext	d0
	bpl.s	raw

repeat:	move	d0,d1
	and	#$3f,d0		; size
	btst	#6,d1		; test bit 6
	beq.s	short
	move.b	(a0)+,d1	; offset
	bra.s	copy

short:	or	#$fff0,d1	; offset
	lsr	#4,d0		; size

copy:	addq	#3,d0
	lea	(a1,d1.w),a3	; w+offset

loop:	move.b	(a3)+,(a1)+
	dbra	d0,loop
	bra.s	test

raw:	move.b	(a0)+,(a1)+
	dbra	d0,raw
test:	cmp.l	a2,a0
	bne.s	next
