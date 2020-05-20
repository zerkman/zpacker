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
	moveq	#0,d0
	move.b	(a0)+,d0
	moveq	#$ffffffc0,d1
	eor	d0,d1
	lsr.b	#6,d1
	bne.s	offset

raw:	and	#$3f,d0		; size
rawlp:	move.b	(a0)+,(a1)+
	dbra	d0,rawlp
	bra.s	test

offset:	moveq.l	#-1,d1
	move.b	(a0)+,d1	; offset
	lea	(a1,d1.w),a3
	addq	#3,d0		; size

offlp:	move.b	(a3)+,(a1)+
	dbra	d0,offlp

test:	cmp.l	a2,a0
	bne.s	next
