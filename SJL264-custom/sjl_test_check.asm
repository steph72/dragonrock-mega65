;-----------------------------------------------------------------------------
;---	SJL264 test data checker
;-----------------------------------------------------------------------------
	cpu	6502
	page	0,0
;-----------------------------------------------------------------------------
def_testing	=	"N"		;If "Y", built test mode

	INCLUDE "_system.ain"		;System Constants
;-----------------------------------------------------------------------------

start_addr	=	$1001


	ORG	start_addr - 2
	ADR	start_addr



	ADR	+, 2017
	BYT	$9e		; sys token
	BYT	"4109"		; $100d
+	BYT	0,0,0

	;ORG	$100d
		jsr	rom_primm
		BYT	$0d,"SJL264 TEST DATA CHECKER",$0d
    IF (def_testing = "Y")
		BYT	"TESTRELEASE",$0d
    ENDIF
		BYT	0
		jsr	unit_selector
		sta	$df
		and	#$03
		asl	a
		tax
		lda	.unitlist,x
		sta	.loadstring+16
		lda	.unitlist+1,x
		sta	.loadstring+17

		lda	#0
		sta	loaderror_no+0
		sta	loaderror_no+1
		sta	ldaddrerror_no+0
		sta	ldaddrerror_no+1

.cycle		jsr	rom_primm
		BYT	$0d,"ERASE MEM...",0
		jsr	memory_fill
		jsr	rom_primm
.loadstring	BYT	$0d,"LOAD TESTDATA U?? (",0
		lda	loaderror_no+1
		jsr	byteprint
		lda	loaderror_no+0
		jsr	byteprint
    IF (def_testing = "Y")
		lda	#"/"
		jsr	rom_chrout
		lda	ldaddrerror_no+1
		jsr	byteprint
		lda	ldaddrerror_no+0
		jsr	byteprint
    ENDIF
		lda	#")"
		jsr	rom_chrout
		jsr	load_testdata
		jsr	rom_primm
		BYT	$0d,"CHECK TESTDATA...",0
		jsr	check_testdata
		cpx	#0
		bne	.error
		cpy	#0
		beq	.okay
.error		tya
		jsr	byteprint
		txa
		jsr	byteprint
		inc	loaderror_no+0
		bne	.cycle
		inc	loaderror_no+1
.cyclejump	jmp	.cycle
.okay
    IF (def_testing = "Y")
		lda	$e6
		cmp	#(tdstart_addr & $ff)
		bne	.error2
		lda	$e7
		cmp	#(tdstart_addr / 256)
		beq	.okay2
.error2		jsr	rom_primm
		BYT	"LDADDR ERR",0
		inc	ldaddrerror_no+0
		bne	.cyclejump
		inc	ldaddrerror_no+1
		jmp	.cycle
.okay2
    ENDIF
		jsr	rom_primm
		BYT	"OKAY!",0
		jmp	.cycle

.unitlist	BYT	" 8 91011"

check_testdata

mode		=	"C"		;Check
	INCLUDE	"sjl_test.ain"
		rts

byteprint	sta	$d0
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		and	#%00001111
		tay
		lda	.numbers,y
		jsr	rom_chrout
		lda	$d0
		and	#%00001111
		tay
		lda	.numbers,y
		jmp	rom_chrout

.numbers	BYT	"0123456789ABCDEF"



load_testdata

		lda	#1
		ldx	$df
    IF (def_testing = "Y")
		ldy	#0			;Test mode: direct start address
    ELSE
		ldy	#1			;Normal mode: start address from file
    ENDIF
		jsr	rom_setlfs
		lda	#(.filename_end-.filename)
		ldx	#(.filename & $ff)
		ldy	#(.filename / 256)
		jsr	rom_setnam
		lda	#0
		ldx	#(tdstart_addr & $ff)
		ldy	#(tdstart_addr / 256)
		jsr	rom_load
		rts

.filename	BYT	"TESTDATA"
.filename_end

loaderror_no	ADR	$0000
ldaddrerror_no	ADR	$0000



memory_fill	ldx	#(tdstart_addr & $ff)
		ldy	#(tdstart_addr / 256)
		stx	$d0
		sty	$d1
		ldy	#0
		lda	#0
.cycle		sta	($d0),y
		inc	$d0
		bne	.lchk
		inc	$d1
.lchk		ldx	$d0
		cpx	#((tdend_addr + 1) & $ff)
		bne	.cycle
		ldx	$d1
		cpx	#((tdend_addr + 1) / 256)
		bne	.cycle
		rts

unit_selector	jsr	rom_primm
		BYT	"CHOOSE UNIT.NO:"
		BYT	$0d,"8: UNIT 8"
		BYT	$0d,"9: UNIT 9"
		BYT	$0d,"A: UNIT 10"
		BYT	$0d,"B: UNIT 11",0
		lda	#0
		sta	z_sfdx
.cycle		ldx	z_sfdx
		lda	#8
		cpx	#$1b			;"8"?
		beq	.ok
		lda	#9
		cpx	#$20			;"9"?
		beq	.ok
		lda	#10
		cpx	#$0a			;"A"?
		beq	.ok
		lda	#11
		cpx	#$1c			;"B"?
		beq	.ok
		jmp	.cycle
.ok		rts



;-----------------------------------------------------------------------------
