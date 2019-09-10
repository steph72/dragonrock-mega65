.export _installIRQ
.export _setSplitEnable

.import         SCNKEY

.segment "LOWCODE"

.code

romsel = $ff3e
ramsel = $ff3f

tedconf1 = $ff07

tedctl1 = $ff12
tedctl2 = $ff13
tedirq  = $ff09
tedirqenable = $ff0a
tedraster = $ff0b

tedborder = $ff19
tedbg = $ff15

irqvec = $314

topRow      = 1
bottomRow   = 137

_installIRQ:        ldx irqvec
                    ldy irqvec+1
                    stx oldvec
                    sty oldvec+1
                    lda #$00
                    sta splitflag   ; initially disable split
                    
                    sei

                    ldx #<newVec
                    ldy #>newVec
                    stx irqvec
                    sty irqvec+1

                    lda #topRow
                    sta tedraster

                    lda #$02
                    sta tedirqenable

                    cli

                    rts

_setSplitEnable:    sta splitflag
                    bne back
                    lda #$f8            ; reset to text mode if splitflag=0
                    sta tedctl2 
back:               rts


newVec:            

                    lda tedirq
                    sta tedirq      ; clear ted interrupt register
                    lda tedraster   ; get raster interrupt position
                    cmp #bottomRow  ; and compare to bottom of graphics position
                    bcc topPos      ; no? has to be top position
                    lda #topRow  
                    sta tedraster   ; we're at the bottom, so reconfigure irq to top

                    lda splitflag
                    beq skip1       ; skip configuring ted if splitflag=0

                    nop
                    nop
                    sta tedborder
                    lda #$f8        ; switch to lowercase/text mode
                    sta tedctl2
                    lda tedconf1
                    and #%01111111
                    sta tedconf1    ; select 128 characters + rvs mode

skip1:              jsr $ff9f       ; perform SCNKEY
                    jsr $ffea       ; perform UDTIM
                    jmp continue

topPos:             lda #bottomRow  ; we're at the top, so reconfigure irq to bottom
                    sta tedraster

                    lda splitflag   ; do we want a split at all?
                    beq continue    ; nope -> continue
                    
                    lda #$05
                    sta tedborder   ; color border for debugging
                    lda #$f0        ; switch to graphics charset
                    sta tedctl2
                    lda tedconf1
                    ora #%10000000   ; enable 256 characters
                    sta tedconf1


continue:           pla             ; ROM interrupt starts with saving registers
                    tay             ; so we have to pull 1 time
                    pla
                    tax
                    pla

                    rti

oldvec:             .word 0
splitflag:          .byte 0