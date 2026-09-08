; da65 V2.16 - Ubuntu 2.16-2
; Created:    2018-09-08 22:49:15
; Input file: example1.o
; Page:       1


        .setcpu "6502"

L016C           := $016C
L0402           := $0402
L056C           := $056C
L1300           := $1300
L1F00           := $1F00
L202D           := $202D
L2076           := $2076
L6269           := $6269
L6E6F           := $6E6F
        eor     $7A,x
        ror     $1161
        brk
        ora     ($00,x)
        rts

        brk
        brk
        brk
        .byte   $0B
        brk
        brk
        brk
        .byte   $6B
        brk
        brk
        brk
        ora     a:$00,x
        brk
        dey
        brk
        brk
        brk
        dec     a:$00,x
        brk
        ror     $01
        brk
        brk
        clc
        brk
        brk
        brk
        ror     a:$01,x
        brk
        .byte   $0C
        brk
        brk
        brk
        txa
        ora     ($00,x)
        brk
        adc     a:$00,x
        brk
        .byte   $1A
        .byte   $02
        brk
        brk
        tax
        brk
        brk
        brk
        cpy     $02
        brk
        brk
LFBFB:  .byte   $37
        ora     ($00,x)
LFBFE:  brk
        .byte   $FB
        .byte   $03
        brk
        brk
        ora     ($00,x)
        brk
        brk
        .byte   $07
        .byte   $02
        brk
        brk
        .byte   $13
        brk
        brk
        brk
        .byte   $FC
        .byte   $03
        brk
        brk
        eor     a:$00
        brk
        .byte   $03
        .byte   $02
        .byte   $02
        rti

        cpx     #$C2
        bne     LFBFB
        ora     $03
        .byte   $03
        .byte   $04
        ora     ($5C,x)
        and     ($94,x)
        .byte   $5B
        txa
        asl     a
        bpl     LFBFE
        .byte   $E2
        ora     $C85A
        .byte   $14
        ora     ($4C),y
        and     ($94,x)
        .byte   $5B
        nop
        .byte   $02
        .byte   $12
        .byte   $37
        lda     $AF,x
        cli
        sbc     ($2C),y
        asl     $98
        brk
        brk
        brk
        jsr     L1F00
        ora     ($02,x)
        .byte   $14
        brk
        .byte   $02
        lda     #$64
        .byte   $02
        .byte   $02
        php
        brk
        ora     ($8D,x)
        .byte   $02
        .byte   $0C
        php
        asl     a
        .byte   $83
        .byte   $02
        .byte   $02
        .byte   $0C
        php
        brk
        .byte   $02
        lda     #$02
        .byte   $02
        ora     #$0F
        brk
        ora     ($8D,x)
        .byte   $02
        .byte   $12
        .byte   $0F
        asl     a
        ora     ($83,x)
        .byte   $02
        sta     ($01,x)
        brk
        brk
        brk
        .byte   $02
        .byte   $12
        .byte   $0F
        brk
        ora     ($AD,x)
        .byte   $02
        ora     $0A1A,x
        .byte   $83
        .byte   $02
        .byte   $02
        ora     a:$1A,x
        ora     ($20,x)
        .byte   $02
        bpl     LFCA2
        asl     a
        .byte   $82
        ora     ($02,x)
        bpl     LFCA8
        brk
        ora     ($AD,x)
        .byte   $02
        ora     $1A
        asl     a
        ora     ($83,x)
        .byte   $02
        sta     ($01,x)
        brk
        brk
        brk
        .byte   $02
        ora     $1A
        brk
        .byte   $01
LFCA2:  jsr     L0402
        .byte   $1A
        asl     a
        .byte   $82
LFCA8:  brk
        .byte   $02
        .byte   $04
        .byte   $1A
        brk
        ora     ($8D,x)
        .byte   $02
        .byte   $14
        .byte   $1A
        asl     a
        ora     ($83,x)
        .byte   $02
        sta     ($02,x)
        brk
        brk
        brk
        .byte   $02
        .byte   $14
        .byte   $1A
        brk
        ora     ($20,x)
        .byte   $02
        asl     a
        asl     $0A,x
        .byte   $82
        .byte   $02
        .byte   $02
        asl     a
        asl     $00,x
        ora     ($4C,x)
        .byte   $02
        ora     $0A1C
        ora     ($83,x)
        brk
        sta     ($1C,x)
        brk
        brk
        brk
        .byte   $02
        ora     $061C
        brk
        brk
        brk
        and     ($00,x)
        brk
        ora     ($02,x)
        brk
        ora     $00,x
        brk
        brk
        .byte   $22
        brk
        .byte   $03
        ora     ($02,x)
        .byte   $03
        brk
        ora     ($00,x)
        ora     ($1B,x)
        brk
        ora     ($00,x)
        ora     ($18,x)
        brk
        ora     ($00,x)
        ora     ($00,x)
        asl     $00
        brk
        brk
        .byte   $23
        brk
        brk
        ora     ($02,x)
        brk
        asl     $00
        brk
        brk
        bit     $00
        brk
        ora     ($01,x)
        brk
        asl     $00
        brk
        brk
        and     $00
        brk
        ora     ($02,x)
        brk
        .byte   $04
        .byte   $02
        asl     $1501,x
        ora     ($04,x)
        .byte   $02
        ora     $1501,x
        ora     ($10,x)
        .byte   $02
        asl     $01,x
        .byte   $0B
        ora     ($0A,x)
        .byte   $02
        .byte   $13
        ora     ($17,x)
        brk
        ora     ($B8,x)
        ora     ($02,x)
        .byte   $17
        .byte   $83
        brk
        .byte   $1F
        ora     ($06,x)
        ora     ($07,x)
        php
        sec
        .byte   $02
        ora     ($1F,x)
        ora     ($83,x)
        brk
        sta     ($1C,x)
        brk
        brk
        brk
        .byte   $03
        .byte   $02
        ora     $011C
        ora     $0290
        .byte   $02
        ora     ($1E,x)
        brk
        brk
        ora     ($15,x)
        ora     ($04,x)
        bcc     LFD63
        .byte   $02
        .byte   $01
LFD63:  ora     $0100,x
        ora     ($15,x)
        ora     ($10,x)
        sec
        .byte   $02
        brk
        .byte   $1B
        ora     ($83,x)
        .byte   $02
        sta     ($02,x)
        brk
        brk
        brk
        brk
        ora     ($0E,x)
        ora     ($14,x)
        sec
        .byte   $02
        brk
        .byte   $1A
        ora     ($83,x)
        .byte   $02
        sta     ($01,x)
        brk
        brk
        brk
        brk
        ora     ($19,x)
        .byte   $02
        .byte   $12
        ora     $38
        .byte   $02
        brk
        clc
        .byte   $83
        .byte   $02
        brk
        ora     ($13,x)
        .byte   $02
        .byte   $0C
        ora     $01B8,x
        .byte   $02
        brk
        .byte   $17
        .byte   $83
        brk
        .byte   $1F
        brk
        ora     ($06,x)
        ora     ($07,x)
        bcc     LFDAA
        .byte   $02
        brk
LFDAA:  asl     $00,x
        .byte   $02
        ora     ($0B,x)
        ora     ($0A,x)
        .byte   $02
        sta     $1401,y
        .byte   $07
        ora     $00,x
        tya
        ora     ($1C,x)
        asl     $15
        ora     ($02,x)
        brk
        brk
        ora     ($01,x)
        brk
        .byte   $1F
        .byte   $02
        ora     ($12),y
        brk
        ora     ($03,x)
        .byte   $02
        .byte   $17
        .byte   $1F
        asl     $01
        ora     ($1E),y
        asl     a:$01,x
        brk
        ora     ($02,x)
        .byte   $0B
        ora     ($00,x)
        brk
        brk
        bmi     LFDE0
        brk
LFDE0:  brk
        ora     ($03,x)
        eor     a:$00
        brk
        brk
        .byte   $3F
        ora     ($00,x)
        brk
        ora     ($0C,x)
        rol     a:$01,x
        brk
        ora     ($0B,x)
        rol     $00
        brk
        brk
        brk
        .byte   $12
        ora     ($00,x)
        brk
        brk
        .byte   $0C
        brk
        .byte   $02
        ora     ($01,x)
        ora     $36
        ora     ($00,x)
        brk
        ora     ($06,x)
        eor     $01
        brk
        brk
        ora     ($0F,x)
        ora     ($01),y
        brk
        brk
        brk
        and     ($01),y
        brk
        brk
        ora     ($04,x)
        lsr     a
        brk
        brk
        brk
        ora     ($10,x)
        ora     a:$00,x
        brk
        brk
        ora     $0200
        ora     ($01,x)
        php
        and     a:$01,x
        brk
        ora     ($0A,x)
        asl     a
        ora     ($00,x)
        brk
        brk
        .byte   $37
        ora     ($00,x)
        brk
        ora     ($07,x)
        .byte   $17
        brk
        brk
        brk
        brk
        rti

        ora     ($00,x)
        brk
        ora     ($0D,x)
        lsr     a:$00
        brk
        brk
        .byte   $0F
        brk
        .byte   $02
        ora     ($01,x)
        .byte   $0F
        .byte   $0F
        ora     ($00,x)
        brk
        brk
        .byte   $1B
        ora     ($00,x)
        brk
        ora     ($01,x)
        .byte   $1A
        brk
        brk
        brk
        brk
        asl     $0200
        ora     ($01,x)
        asl     $0118
        brk
        brk
        ora     ($00,x)
        bpl     LFE71
LFE71:  .byte   $02
        ora     ($01,x)
        bpl     LFEB2
        ora     ($00,x)
        brk
        ora     ($09,x)
        rol     $00
        asl     a
        adc     $78
        adc     ($6D,x)
        bvs     LFEF0
        adc     $31
        rol     $1A73
        .byte   $63
        adc     ($36,x)
        and     $20,x
        lsr     $32,x
        rol     $3631
        jsr     L202D
        eor     $62,x
        adc     $6E,x
        .byte   $74
        adc     $20,x
        .byte   $32
        rol     $3631
        and     $1B32
        .byte   $63
        .byte   $63
        rol     $35,x
        jsr     L2076
        .byte   $32
        rol     $3631
        jsr     L202D
LFEB2:  eor     $62,x
        adc     $6E,x
        .byte   $74
        adc     $20,x
        .byte   $32
        rol     $3631
        and     $0232
        .byte   $73
        bvs     LFEC7
        .byte   $73
        .byte   $72
        adc     $67
LFEC7:  .byte   $07
        .byte   $72
        adc     $67
        .byte   $73
        adc     ($76,x)
        adc     $07
        .byte   $72
        adc     $67
        .byte   $62
        adc     ($6E,x)
        .byte   $6B
        .byte   $04
        .byte   $74
        adc     $3170
        .byte   $04
        .byte   $74
        adc     $3270
        .byte   $04
        .byte   $74
        adc     $3370
        .byte   $04
        .byte   $74
        adc     $3470
        .byte   $04
        bvs     LFF62
        .byte   $72
        .byte   $31
LFEF0:  .byte   $04
        bvs     LFF67
        .byte   $72
        .byte   $32
        .byte   $04
        bvs     LFF6C
        .byte   $72
        .byte   $33
        .byte   $04
        bvs     LFF71
        .byte   $72
        .byte   $34
        and     $2F
        adc     $73,x
        .byte   $72
        .byte   $2F
        .byte   $73
        pla
        adc     ($72,x)
        adc     $2F
        .byte   $63
        .byte   $63
        rol     $35,x
        .byte   $2F
        adc     ($73,x)
        adc     $6E69
        .byte   $63
        .byte   $2F
        jmp     (L6E6F)

        .byte   $67
        .byte   $62
        .byte   $72
        adc     ($6E,x)
        .byte   $63
        pla
        rol     $616D
        .byte   $63
        asl     a
        adc     $78
        adc     ($6D,x)
        bvs     LFF98
        adc     $31
        rol     $0863
        ror     $7365
        jmp     (L6269)

        rol     $0B68
        .byte   $5F
        .byte   $5F
        .byte   $53
        .byte   $54
        eor     ($52,x)
        .byte   $54
        eor     $50,x
        .byte   $5F
        .byte   $5F
        asl     a
        bvs     LFFB8
        adc     $5F,x
        .byte   $6F
        ror     $615F
        jmp     (L016C)

        brk
        .byte   $0B
        .byte   $5F
        bvs     LFFC6
        adc     $5F,x
        .byte   $6F
        ror     $615F
        jmp     (L056C)

        .byte   $5F
        .byte   $6D
        .byte   $61
LFF62:  adc     #$6E
        .byte   $02
        .byte   $5F
        sei
LFF67:  ora     $2E
        .byte   $73
        adc     #$7A
LFF6C:  adc     $02
        .byte   $5F
        .byte   $79
        .byte   $02
LFF71:  .byte   $5F
        .byte   $7A
        .byte   $04
        adc     $6961
        ror     $7006
        adc     $73,x
        pla
        adc     ($30,x)
        ora     #$74
        .byte   $6F
        .byte   $73
        adc     $64,x
        adc     #$76
        adc     ($30,x)
        ora     $4C
        bmi     LFFBD
        bmi     LFFD3
        .byte   $04
        .byte   $43
        .byte   $4F
        .byte   $44
        eor     $06
        .byte   $52
        .byte   $4F
        .byte   $44
LFF98:  eor     ($54,x)
        eor     ($03,x)
        .byte   $42
        .byte   $53
        .byte   $53
        .byte   $04
        .byte   $44
        eor     ($54,x)
        eor     ($08,x)
        .byte   $5A
        eor     $52
        .byte   $4F
        bvc     LFFEC
        .byte   $47
        eor     $04
        lsr     $4C55
        jmp     L1300

        .byte   $02
        brk
        ora     ($00,x)
LFFB8:  .byte   $02
        ora     ($01,x)
        brk
        .byte   $02
LFFBD:  .byte   $02
        ora     ($00,x)
        brk
        brk
        .byte   $02
        brk
        brk
        .byte   $02
LFFC6:  .byte   $03
        brk
        brk
        brk
        ora     $00
        brk
        ora     $02
        brk
        brk
        .byte   $07
        .byte   $03
LFFD3:  brk
        brk
        ora     $05
        brk
        brk
        asl     a
        .byte   $03
        brk
        brk
        ora     a:$03
        brk
        bpl     LFFE6
        brk
        brk
        .byte   $13
LFFE6:  .byte   $03
        brk
        brk
        asl     $03,x
        brk
LFFEC:  brk
        asl     a
        .byte   $0F
        brk
        brk
        ora     $03,y
        brk
        .byte   $1C
        .byte   $03
        brk
        brk
        brk
        .byte   $1F
        brk
        .byte   $02
        brk
        .byte   $03
        brk
