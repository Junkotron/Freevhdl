.org $C000

RESET_START:
    SEI
    CLD
    LDX #$FF
    TXS

    ; Skriv till VIA för att sätta igång Timer 1 (ACR = $030B, IER = $030E)
    LDA #$C0
    STA $030E
    LDA #$40
    STA $030B

    ; Starta timern genom att ladda värdet 5 (T1C-L = $0304, T1C-H = $0305)
    LDA #$05
    STA $0304
    LDA #$00
    STA $0305

    CLI
	
LOOP_FOREVER:
    JMP LOOP_FOREVER

; 🕵️‍♂️ Vår riktiga Interrupt Service Routine (ISR)
IRQ_HANDLER:
    PHA             ; Spara A på stacken
    TXA
    PHA             ; Spara X på stacken

    ; Kvittera avbrottet i VIA genom att läsa Timer 1 Low Counter ($0304) 
    ; eller IFR ($030D) så att VIA släpper ner avbrottspinnen igen!
    LDA $0304       

    PLA
    TAX             ; Återställ X
    PLA             ; Återställ A
    RTI             ; Return from Interrupt - hoppar tillbaka till där vi var!

; Padding fram till vektorerna
.dsb $FFFA - *, $EA

; Vektorerna placeras på sina fysiska platser i slutet av 16KB-rymden
.word $0000             ; NMI-vektor ($FFFA-$FFFB)
.word RESET_START       ; Reset-vektor ($FFFC-$FFFD)
.word IRQ_HANDLER       ; IRQ-vektor ($FFFE-$FFFF) - Peka på vår rutin!
