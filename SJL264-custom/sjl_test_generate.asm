;-----------------------------------------------------------------------------
;---	SJL264 test data generator
;-----------------------------------------------------------------------------
	cpu	6502
	page	0,0
;-----------------------------------------------------------------------------
	INCLUDE "_system.ain"		;System Constants
;-----------------------------------------------------------------------------

start_addr	=	$1001


	ORG	start_addr - 2
	ADR	start_addr



	ADR	+, 2017
	BYT	$9e		; sys token
	BYT	"4109"		; $100d
+	BYT	0,0,0

	ORG	$100d
		jsr	rom_primm
		BYT	$0d,"SJL264 TEST DATA GENERATOR",$0d
		BYT	"GENERATING DATAS...",$0d,0

mode		=	"G"		;Generate
	INCLUDE	"sjl_test.ain"

		jsr	rom_primm
		BYT	$0d,"DATA GENERATED. ",0
		jsr	unit_selector
		sta	$e0
		and	#$03
		asl	a
		tax
		lda	.unitlist,x
		sta	.savestring+13
		lda	.unitlist+1,x
		sta	.savestring+14
		jsr	rom_primm
.savestring	BYT	$0d,"SAVE TO UNIT??...",$0d,0
		lda	#1
		ldx	$e0
		ldy	#0
		jsr	rom_setlfs
		lda	#(.filename_end-.filename)
		ldx	#(.filename & $ff)
		ldy	#(.filename / 256)
		jsr	rom_setnam
		ldx	#(tdstart_addr & $ff)
		ldy	#(tdstart_addr / 256)
		stx	$d0
		sty	$d1
		lda	#$d0
		ldx	#((tdend_addr + 1) & $ff)
		ldy	#((tdend_addr + 1) / 256)
		jsr	rom_save

		jsr	rom_primm
		BYT	$0d,"DATA SAVED.",$0d,0
		rts

.filename	BYT	"TESTDATA"
.filename_end

.unitlist	BYT	" 8 91011"

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
