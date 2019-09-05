.export _copychars

.code
.importzp ptr1, ptr2

tedctl1 = $ff12
tedctl2 = $ff13

romsel = $ff3e
ramsel = $ff3f

csize     = $800         ; only copy one character set
ramchars  = $f000        ; to the top of available memory
romchars  = $d000        ; lowercase set

gamechars = ramchars + $400 + ($5b*8)

_copychars:

; save context to stack

    pha
    txa
    pha
    tya
    pha

; setup pointers for rom charset copy

    ldx #<romchars
    ldy #>romchars
    stx ptr1
    sty ptr1+1
    ldx #<ramchars
    ldy #>ramchars
    stx ptr2
    sty ptr2+1
    ldx #<csize
    ldy #>csize
    stx sizel
    sty sizeh

    sta romsel
    jsr moveup
    sta ramsel

; setup pointers for gamechars copy

    ldx #<gfx
    ldy #>gfx
    stx ptr1
    sty ptr1+1
    ldx #<gamechars
    ldy #>gamechars
    stx ptr2
    sty ptr2+1
    ldx #40
    ldy #00
    stx sizel
    sty sizeh

    jsr moveup

    lda tedctl1
    and %11111011
    sta tedctl1
    lda #>gamechars
    sta tedctl2

; restore context from stack

    pla
    txa
    pla
    tya
    pla
    rts

moveup:  LDX sizeh    ; the last byte must be moved first
         CLC          ; start at the final pages of FROM and TO
         TXA
         ADC ptr1+1
         STA ptr1+1
         CLC
         TXA
         ADC ptr2+1
         STA ptr2+1
         INX          ; allows the use of BNE after the DEX below
         LDY sizel
         BEQ MU3
         DEY          ; move bytes on the last page first
         BEQ MU2
MU1:     LDA (ptr1),Y
         STA (ptr2),Y
         DEY
         BNE MU1
MU2:     LDA (ptr1),Y ; handle Y = 0 separately
         STA (ptr2),Y
MU3:     DEY
         DEC ptr1+1   ; move the next page (if any)
         DEC ptr2+1
         DEX
         BNE MU1
         RTS

sizel:      .byte 0
sizeh:      .byte 0  

gfx:

            .byte %00000000
            .byte %00000000
            .byte %00100100
            .byte %00011000
            .byte %00011000
            .byte %00100100
            .byte %00000000
            .byte %00000000

            .byte %00001000
            .byte %00001000
            .byte %00001000
            .byte %00001000
            .byte %00001000
            .byte %00001000
            .byte %00001000
            .byte %00001000

            .byte %00000000
            .byte %00000000
            .byte %00000000
            .byte %00000000
            .byte %11111111
            .byte %00000000
            .byte %00000000
            .byte %00000000

            .byte %10101010
            .byte %01010101
            .byte %10101010
            .byte %01010101
            .byte %10101010
            .byte %01010101
            .byte %10101010
            .byte %01010101

            .byte %11111111
            .byte %10011001
            .byte %10011001
            .byte %11111111
            .byte %01011010
            .byte %01011010
            .byte %00111100
            .byte %00011000


