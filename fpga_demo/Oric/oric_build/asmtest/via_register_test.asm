; VIA 6522 REGISTER STATIC RW TEST
.org $C000

RESET_START:
    SEI
    CLD
    LDX #$FF
    TXS

; TEST 1 Skriv och las PCR $030C med monster $55
    LDA #$55
    STA $030C

    LDA #$00
    LDA $030C

; TEST 2 Skriv och las DDRA $0303 med monster $AA
    LDA #$AA
    STA $0303

    LDA #$00
    LDA $0303

LOOP_FOREVER:
    JMP LOOP_FOREVER

; Padding till slutet pa 16KB-blocket
.dsb $FFFC - *, $EA

; Vektorerna stämplas in
.word RESET_START
.word $0000
