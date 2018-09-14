;
; File generated by cc65 v 2.16 - Ubuntu 2.16-2
;
	.fopt		compiler,"cc65 v 2.16 - Ubuntu 2.16-2"
	.setcpu		"6502"
	.smart		on
	.autoimport	on
	.case		on
	.debuginfo	off
	.importzp	sp, sreg, regsave, regbank
	.importzp	tmp1, tmp2, tmp3, tmp4, ptr1, ptr2, ptr3, ptr4
	.macpack	longbranch
	.forceimport	__STARTUP__
	.import		_pal_col
	.import		_ppu_on_all
	.import		_oam_spr
	.import		_ppu_waitnmi
	.import		_pad_poll
	.import		_set_vram_update
	.import		_memcpy
	.export		_list_init
	.export		_main

.segment	"RODATA"

_list_init:
	.byte	$20
	.byte	$42
	.byte	$00
	.byte	$20
	.byte	$43
	.byte	$00
	.byte	$20
	.byte	$44
	.byte	$00
	.byte	$20
	.byte	$46
	.byte	$00
	.byte	$20
	.byte	$47
	.byte	$00
	.byte	$20
	.byte	$48
	.byte	$00

.segment	"BSS"

.segment	"BSS"
_i:
	.res	1,$00
.segment	"BSS"
_x:
	.res	1,$00
.segment	"BSS"
_y:
	.res	1,$00
.segment	"BSS"
_list:
	.res	18,$00

; ---------------------------------------------------------------
; void __near__ main (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_main: near

.segment	"CODE"

;
; pal_col(1,0x21);//blue color for text
;
	lda     #$01
	jsr     pusha
	lda     #$21
	jsr     _pal_col
;
; pal_col(17,0x30);//white color for sprite
;
	lda     #$11
	jsr     pusha
	lda     #$30
	jsr     _pal_col
;
; memcpy(list,list_init,sizeof(list_init));
;
	lda     #<(_list)
	ldx     #>(_list)
	jsr     pushax
	lda     #<(_list_init)
	ldx     #>(_list_init)
	jsr     pushax
	ldx     #$00
	lda     #$12
	jsr     _memcpy
;
; set_vram_update(6,list);
;
	lda     #$06
	jsr     pusha
	lda     #<(_list)
	ldx     #>(_list)
	jsr     _set_vram_update
;
; ppu_on_all();//enable rendering
;
	jsr     _ppu_on_all
;
; x=124;
;
	lda     #$7C
	sta     _x
;
; y=116;
;
	lda     #$74
	sta     _y
;
; ppu_waitnmi();//wait for next TV frame
;
L007D:	jsr     _ppu_waitnmi
;
; oam_spr(x,y,0x41,0,0);//put sprite
;
	jsr     decsp4
	lda     _x
	ldy     #$03
	sta     (sp),y
	lda     _y
	dey
	sta     (sp),y
	lda     #$41
	dey
	sta     (sp),y
	lda     #$00
	dey
	sta     (sp),y
	jsr     _oam_spr
;
; i=pad_poll(0);
;
	lda     #$00
	jsr     _pad_poll
	sta     _i
;
; if(i&PAD_LEFT &&x>  0) x-=2;
;
	and     #$40
	beq     L00BE
	lda     _x
	beq     L00BE
	sec
	sbc     #$02
	sta     _x
;
; if(i&PAD_RIGHT&&x<248) x+=2;
;
L00BE:	lda     _i
	and     #$80
	beq     L00C2
	lda     _x
	cmp     #$F8
	bcs     L00C2
	lda     #$02
	clc
	adc     _x
	sta     _x
;
; if(i&PAD_UP   &&y>  0) y-=2;
;
L00C2:	lda     _i
	and     #$10
	beq     L00C6
	lda     _y
	beq     L00C6
	sec
	sbc     #$02
	sta     _y
;
; if(i&PAD_DOWN &&y<232) y+=2;
;
L00C6:	lda     _i
	and     #$20
	beq     L00CA
	lda     _y
	cmp     #$E8
	bcs     L00CA
	lda     #$02
	clc
	adc     _y
	sta     _y
;
; list[2]=0x10+x/100;
;
L00CA:	lda     _x
	jsr     pusha0
	lda     #$64
	jsr     tosudiva0
	clc
	adc     #$10
	sta     _list+2
;
; list[5]=0x10+x/10%10;
;
	lda     _x
	jsr     pusha0
	lda     #$0A
	jsr     tosudiva0
	jsr     pushax
	lda     #$0A
	jsr     tosumoda0
	clc
	adc     #$10
	sta     _list+5
;
; list[8]=0x10+x%10;
;
	lda     _x
	jsr     pusha0
	lda     #$0A
	jsr     tosumoda0
	clc
	adc     #$10
	sta     _list+8
;
; list[11]=0x10+y/100;
;
	lda     _y
	jsr     pusha0
	lda     #$64
	jsr     tosudiva0
	clc
	adc     #$10
	sta     _list+11
;
; list[14]=0x10+y/10%10;
;
	lda     _y
	jsr     pusha0
	lda     #$0A
	jsr     tosudiva0
	jsr     pushax
	lda     #$0A
	jsr     tosumoda0
	clc
	adc     #$10
	sta     _list+14
;
; list[17]=0x10+y%10;
;
	lda     _y
	jsr     pusha0
	lda     #$0A
	jsr     tosumoda0
	clc
	adc     #$10
	sta     _list+17
;
; while(1)
;
	jmp     L007D

.endproc

