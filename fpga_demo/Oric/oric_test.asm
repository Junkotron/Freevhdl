; Korrekt 64tass-syntax utan dolda fällor

        * = $C000       ; Sätt startadressen till $C000 med stjärna

start:
        lda #$99        ; Ladda testvärde 0x99
        sta $0100       ; Skriv till RAM-cell $0100
        lda #$00        ; Rensa ackumulatorn
        lda $0100       ; Läs tillbaka från RAM
        cmp #$99        ; Jämför: Fick vi tillbaka 0x99?
        beq ram_ok      ; JA: Hoppa till framgångs-loopen

fail:
        jmp fail        ; NEJ: Fastna i FAIL-loop

ram_ok:
        jmp ram_ok      ; SUCCESS-loop!

; --- Fyll ut exakt fram till slutet av den 16 KB stora filen ---
; .fill tar <antal bytes>, <fyllnadsvärde>. 
; Vi räknar ut utrymmet fram till $FFFC genom att dra av nuvarande position (*).
        .fill $FFFC - *, 0

        .word start     ; $FFFC-$FFFD: Reset-vektor
        .word fail      ; $FFFE-$FFFF: IRQ-vektor

