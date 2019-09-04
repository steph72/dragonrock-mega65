.export _installIRQ

.code

tedctl1 = $ff12
tedctl2 = $ff13
tedirq  = $ff09
tedirqenable = $ff0a
tedraster = $ff0b
tedborder = $ff19

irqvec = $314

_installIRQ:        ldx irqvec
                    ldy irqvec+1
                    stx oldvec
                    sty oldvec+1
                    
                    sei

                    ldx #<newVec
                    ldy #>newVec
                    stx irqvec
                    sty irqvec+1

                    lda #$30
                    sta tedraster

                    lda #$02
                    sta tedirqenable

                    cli

                    rts

newVec:             pha
                    txa
                    pha
                    tya
                    pha

                    lda tedirq
                    sta tedirq      ; clear ted interrupt register
                    lda tedraster   ; get raster interrupt position
                    cmp #$50        ; and compare to bottom of graphics position
                    bcc topPos      ; no? has to be top position

                    lda #$30  
                    sta tedraster   ; we're at the bottom, so reconfigure irq to top

                    inc tedborder       ; test inc bg
                    jmp continue

topPos:             lda #$50        ; we're at the top, so reconfigure irq to bottom
                    sta tedraster
                    dec tedborder   ; test dec bg

continue:           pla
                    tay
                    pla
                    tax
                    pla

                    rti




oldvec:             .word 0
