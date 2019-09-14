.export _enableCustomCharset
.export _disableCustomCharset

.code

tedctl1 = $ff12
tedctl2 = $ff13

gamechars = $f800

_enableCustomCharset:        lda tedctl1
                             and #%11111011
                             sta tedctl1
                             lda #>gamechars
                             sta tedctl2
                             rts

_disableCustomCharset:       lda tedctl1
                             ora #%00000100
                             sta tedctl1
                             lda #$d4
                             sta tedctl2
                             rts

