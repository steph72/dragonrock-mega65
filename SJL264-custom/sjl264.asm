;-----------------------------------------------------------------------------
;---	SJL264 by BSZ
;---	C16, C116, plus/4 FastLoader for JiffyDOS equipped devices
;---	Based on C64's SJLOAD software:
;---		https://www.c64-wiki.com/wiki/SJLOAD
;---	Thanks to the original authors!
;---		2017.08.15.	Start!
;---		2017.09.04.	First load check
;---		2017.09.10.	First load check on real HW, it's work! :)
;---		2017.09.11.	Create useable program; support $0A00..$FAEF (..$FCFF) area for load
;---		2017.11.01.	Modify "Working Indicator", Add On/Off switch
;---		2017.11.05.	RC1
;---		2017.11.19.	Release 0.2
;---		2018.01.07.	Release 0.3, 16K/32K support, DIRECTORY, $0A00..$FAFF, ...
;-----------------------------------------------------------------------------
	cpu	6502
	page	0,0
;-----------------------------------------------------------------------------
def_version	=	"0.3"

def_testing	=	"N"		;If "Y", built test mode
def_betatext	=	""		;If not empty string, build beta/rc release (only display _this_ text)
def_wimode	=	3		;0,1,2,3 Working indicator mode (0: none, 1, 2, 3: Border color changes)
def_onoffswitch	=	"Y"		;If "Y", enable On/Off switch

	INCLUDE "_system.ain"		;System Constants
;-----------------------------------------------------------------------------
start_addr	=	$1001		;Loader system start address
switcher_addr	=	$05c6		;On/Off switch routine start address (V0.3+, Funct.key buffer)
bootstrap_addr	=	$05f5		;Bootstrap start address
ldr_addr	=	$0801		;Main loader routine start address
ldrstore16_addr	=	$3e00		;Main loader routine storage area / 16K RAM
ldrstore32_addr	=	$7e00		;Main loader routine storage area / 32K RAM
ldrstore64_addr	=	$ee00		;Main loader routine storage area / 64K RAM
;-----------------------------------------------------------------------------

	ORG	start_addr - 2
	ADR	start_addr

;	BASIC area:
	ADR	+, 2017
	BYT	$9e		; sys token
	BYT	"4109"		; $100d
+	BYT	0,0,0



;	Init routine:
	ORG	$100d
		lda	$01
		and	#%00001000		;Cass. Motor on?
		bne	.mtroff
		jsr	rom_primm
		BYT	$0d,"PLEASE DISCONNECT DATASETTE!",$0d,0
		rts
.mtroff

;	Patch copy routines for 16K/32K/64K, set memory:
		lda	b_memsiz+1			;Basic FREE memory END
		ldx	#(ldrstore16_addr / 256)	;16K memory
		cmp	#$40
		bcc	.memsizeset
		ldx	#(ldrstore32_addr / 256)	;32K memory
		cmp	#$80
		bcc	.memsizeset
		ldx	#(ldrstore64_addr / 256)	;64K memory
.memsizeset	stx	b_memsiz+1
		lda	#(ldrstore64_addr & $ff)	;Low Byte is equal in 16K/32K/64K
		sta	b_memsiz+0
		stx	.loadccopy1_wr+2		;Patch the program -> "save" copy routine 1
		stx	load_routine.loadccopy1-bootstrap_addr+bootstrap_code_start+2
		inx
		stx	.loadccopy2_wr+2		;Patch the program -> "save" copy routine 2 
		stx	load_routine.loadccopy2-bootstrap_addr+bootstrap_code_start+2

;	Copy codes to "storage" area:
		ldx	#bootstrap_code_length		;Bootstrap copy
.bootstrapccopy	lda	bootstrap_code_start-1,x
		sta	bootstrap_addr-1,x
		dex
		bne	.bootstrapccopy

		lda	#$5a
		sta	z_bsour				;CkSum init
		ldy	#0
.loadccopy1	lda	loader_code_start,y
.loadccopy1_wr	sta	ldrstore64_addr,y		;Address patched to 16K/32K/64K
		eor	z_bsour
		sta	z_bsour
		iny
		bne	.loadccopy1
.loadccopy2	lda	loader_code_start+256,y
.loadccopy2_wr	sta	ldrstore64_addr+256,y		;Address patched to 16K/32K/64K
		eor	z_bsour
		sta	z_bsour
		iny
		cpy	#loader_code_length & $ff
		bne	.loadccopy2
		sta	cksum_check+1			;Patch CkSum checker

;	Prepare On/Off switch routine (if required):
    IF (def_onoffswitch = "Y")
		lda	#0				;"F1" Function Key
		sta	$76
		lda	#.functionkeystring_end - .functionkeystring
		ldx	#(.functionkeystring & $ff)
		ldy	#(.functionkeystring / 256)
		stx	$22
		sty	$23
		jsr	rom_key				;Set string to "F1" key
		ldx	#.switcherroutine_end - .switcherroutine - 1
.switchercopy	lda	.switcherroutine,x
		sta	switcher_addr,x
		dex
		bpl	.switchercopy
    ENDIF

;	Set load vector, reset BASIC interpreter:
		lda	#(load_routine & $ff)
		sta	v_iload+0
		lda	#(load_routine / 256)
		sta	v_iload+1
		jsr	bas_vectors_init		;$8117
		jsr	bas_reset			;$802e
		lda	z_txttab+0
		ldy	z_txttab+1
		jsr	bas_memory_check		;$8923
		cli
		jmp	bas_coldreset_stack		;$8025	Start BASIC Interpreter

    IF (def_onoffswitch = "Y")
.functionkeystring
	BYT	"SYS1525", $0d
.functionkeystring_end

.switcherroutine
	PHASE switcher_addr
		lda	#'D'
		ldx	#(rom_orig_load_vect & $ff)
		ldy	#(rom_orig_load_vect / 256)
		bit	v_iload+1
		bpl	.switch
		lda	#'E'
		ldx	#(load_routine & $ff)
		ldy	#(load_routine / 256)
.switch		sta	.switchchar
		stx	v_iload+0
		sty	v_iload+1
		jsr	rom_primm
.switchchar	BYT	"?",$0d,0
		rts
	DEPHASE
.switcherroutine_end

    ENDIF

;-----------------------------------------------------------------------------
;---	Loader pre/post process routines:

bootstrap_code_start

	PHASE bootstrap_addr

    IF (def_onoffswitch = "Y")
		jmp	switcher_addr		;On/Off switch 
    ELSE
		rts				;3-PLUS-1 start address: Return, if called
    ENDIF

load_routine	sta	z_verfck		;Store Verify flag
		lda	#0
		sta	z_status		;Clear ST
		lda	z_fa			;Selected Device no
		cmp	#8
		bcs	.driveaddr
.origload	jmp	rom_load_cont		;No drive, go to ROM load

.driveaddr	lda	z_fnlen			;FileName length
		beq	.origload		;If 0, go to ROM load
		lda	z_verfck		;Verify flag
		bne	.origload

		ldy	#0			;First character
		lda	#$af			;from FileName
		sta	$07df
		jsr	$07d9
		cmp	#"$"			;Filename is "$"?
		beq	.origload		;If yes, KERNAL load required, Fastloading DIRECTORY not possible

		jsr	rom_searchingfor	;Print "SEARCHING FOR..." text
		jsr	rom_loadingverifying	;Print "LOADING" text
		lda	hw_ted_scrcontrol1
		sta	sjl_retted06+1		;Save original TED register value
		and	#%11101111
		sta	hw_ted_scrcontrol1	;Screen Off

		php
		sei
		sta	$ff3f
		lda	#$5a
		sta	z_bsour			;CkSum init
		ldy	#0
.loadccopy1	lda	ldrstore64_addr,y	;Address patched for proper memory size
		sta	ldr_addr,y
		eor	z_bsour
		sta	z_bsour
		iny
		bne	.loadccopy1
.loadccopy2	lda	ldrstore64_addr+256,y	;Address patched for proper memory size
		sta	ldr_addr+256,y
		eor	z_bsour
		sta	z_bsour
		iny
		cpy	#loader_code_length & $ff
		bne	.loadccopy2
		sta	$ff3e
		plp

		lda	hw_ted_chargenclk
		sta	sjl_retted13+1
		ora	#%00000010
		sta	hw_ted_chargenclk	;Single Clock select

    IF (def_wimode = 3)
		lda	hw_ted_bordercolor
		sta	loader_routine.bordercolor+1
    ELSEIF (def_wimode > 0)
		lda	hw_ted_bordercolor
		sta	sjl_retted19+1
    ENDIF

		lda	z_bsour
cksum_check	cmp	#0			;Self-modifyed: load routine is valid?
		beq	.waitborder
		jsr	sjl_returnrout
		ADR	rom_load_cont		;KERNAL LOAD

.waitborder	lda	hw_ted_ypos
		cmp	#220
		bne	.waitborder
		jmp	loader_routine

;	"RETURN" routine. Restore screen and exit:
sjl_returnrout	php
		pha
		txa
		pha
		tya
		pha

sjl_retted06	lda	#0			;Self-modified data: original TED register value
		sta	hw_ted_scrcontrol1	;Screen restore
sjl_retted13	lda	#0
		sta	hw_ted_chargenclk	;Clock restore

    IF (def_wimode = 3)
    ELSEIF (def_wimode > 0)
sjl_retted19	lda	#0
		sta	hw_ted_bordercolor	;Border color restore
    ENDIF

		lda	#%00001111
		sta	$00			;Restore CPUPORT DDR
		lda	z_eal
		pha
		lda	z_eah
		pha
		tsx
		lda	$0107,x
		sta	z_eal
		lda	$0108,x
		sta	z_eah
		ldy	#1
		lda	(z_eal),y
		sta	$0107,x
		iny
		lda	(z_eal),y
		sta	$0108,x
		ldx	#0
		lda	$0800
.colmemrestore	sta	ldr_addr-1,x		;~restore original colour memory
		sta	ldr_addr-1+256,x
		inx
		bne	.colmemrestore
sjl_erldingaddr	jsr	.eraseloadtext
		pla
		sta	z_eah
		pla
		sta	z_eal
		pla
		tay
		pla
		tax
		pla
		rti

.eraseloadtext	bit	z_msgfgl
		bpl	.eraloadnot
		jsr	rom_primm
		BYT	$1b,"P",$1b,"J",$91,0
.eraloadnot	rts

	DEPHASE

bootstrap_code_length = * - bootstrap_code_start

;-----------------------------------------------------------------------------
;---	Loader routine:

loader_code_start

	PHASE ldr_addr

open_exit	jsr	sjl_returnrout
		ADR	.retaddr
.retaddr	rts

loader_routine	ldx	z_sa			;Get original secondary address from LOAD routine, not IEC SA!
		lda	#$60
		sta	z_sa			;Set secondary address: $60: load from drive channel 0

		lda	#((open_exit-1) / 256)
		pha
		lda	#((open_exit-1) & $ff)
		pha				;"?DEVICE NOT PRESENT ERROR." ~routine
		jsr	rom_iec_open		;BUSOP. If DNP, drop return address and go to error handler
		pla
		pla
		sei

		lda	$01
		and	#%00001000		;Cass. Motor on?
		beq	.filenotfound		;If yes, ERROR
		lda	#%00001000
		sta	$01			;Cass. RD line 0
		lda	#%00011111
		sta	$00			;Dirty Hack: Switch Output to Cass. RD line

		lda	z_fa			;Device no
		jsr	sjl_talk		;SJL Talk
		lda	z_sa			;Secondary address
		jsr	sjl_sectalk		;Secondary Talk
		jsr	sjl_busin		;Get load address Lo BYTE
		sta	z_eal
    IF (def_testing = "Y")
		sta	$e6			;If test, store original LOAD address LO
    ENDIF
		lda	z_status
		lsr	a
		lsr	a
		bcc	.filefound
.filenotfound	jsr	sjl_returnrout
		ADR	rom_error_filenotfnd	;"?FILE NOT FOUND ERROR."

.filefound	jsr	sjl_busin		;Get load address Hi BYTE
		sta	z_eah
    IF (def_testing = "Y")
		sta	$e7			;If test, store original LOAD address HI
    ENDIF
		jsr	sjl_untalk

		txa				;LOAD sec.addr = 0? Original load address / basic load address?
		bne	.origloadaddr
		lda	z_memuss_l
		sta	z_eal
		lda	z_memuss_h
		sta	z_eah
.origloadaddr	lda	z_eah
		cmp	#$0a			;Load address >= $0A00?
		bcs	.ldaddrokay
		jsr	rom_iec_close		;Close file
		ldx	z_sa
		jsr	sjl_returnrout
		ADR	rom_load_driveload	;KERNAL load from disk, original

.ldaddrokay	inc	z_sa			;Secondary address $61 = JD load
		lda	z_fa			;Device no
		jsr	sjl_talk		;SJL Talk
		lda	z_sa			;Secondary address
		jsr	sjl_sectalk		;Secondary Talk
		dec	z_sa

		ldy	#$00			;Mem. write index = always 0
		;ldx	#$00
		ldx	#231
.wait1		dex
		bne	.wait1

.loadloop	lda	#%00001000		;IEC lines released
		sta	$01
		bit	$01
		bvc	*-2			;Wait until 1541 release CLK
		bmi	.loadendover		;Branch if load ended
		bit	$01
		bpl	*-2			;Wait until 1541 release DAT

.transferbyte	nop				;	 Timing critical section
		nop
		nop
		nop
		lda	#%00001000		;	 Release ATN/CLK/DAT
		ldx	#%00001001		;	 Drive DAT
		stx	$01			;(15/16)
		bit	$01			;(18/20)
		bvc	.loadloop		;	 If 1541 activate CLK line, wait next block
		nop
		sta	$01			;(25/28) Release DAT
		lda	$01			;(28/32) Read bits 10
		nop
		lsr	a
		lsr	a
		eor	$01			;(37/42) Read bits 32
		bit	$00
		lsr	a
		lsr	a
		eor	$01			;(47/53) Read bits 54
		bit	$00
		lsr
		lsr
		eor	$01			;(57/64) Read bits 76
		eor	#%00001010		;	 Flip back (cas. mtr) bits
.memwrt_opcode	sta	(z_eal),y		;	 Store received BYTE
		inc	z_eal
		bne	.transferbyte

    IF (def_wimode = 1)
		and	#%00001111
		ora	#%00010000
		eor	hw_ted_bordercolor
		sta	hw_ted_bordercolor	;Border color change: "working indicator"
    ENDIF
    IF (def_wimode = 2)
		and	#%00001111
		sta	hw_ted_bordercolor	;Border color change: "working indicator"
		bne	.notblack
		inc	hw_ted_bordercolor	;Border color: change black to gray
.notblack
    ENDIF
    IF (def_wimode = 3)
		sta	hw_ted_bordercolor	;Border color change: "working indicator"
		eor	#$7f
		sta	hw_ted_bordercolor
.bordercolor	lda	#$00			;Self-modified: Saved Border color
		sta	hw_ted_bordercolor	;Border color restore
    ENDIF

		inc	z_eah
		lda	z_eah
		beq	.memwrtdis		;If next address = $0000, error, disable memwrite
		cmp	#$fd
		bne	.transferbyte		;If next address < $FD00, okay
.memwrtdis	lda	#$b1			;LDA ($ZP),y op.code
		sta	.memwrt_opcode
		bne	.transferbyte		;BRA read cycle

.loadendover	ldx	#$64
.end_check	bit	$01
		bvc	.end_ok
		dex
		bne	.end_check
		lda	#%01000010		;EOI, TimeOut under receiving
		BYT	$2c			;~NOP $xxxx
.end_ok		lda	#%01000000		;EOI
		jsr	rom_set_status
		jsr	sjl_untalk
		jsr	rom_iec_close		;IEC bus CLOSE
		bcc	.end_ready
		jsr	sjl_returnrout
		ADR	rom_error_filenotfnd
.end_ready	lda	#$2c			;BIT $xxxx op.code
		sta	sjl_erldingaddr		;Patch "LOADING" text eraser routine
		jsr	sjl_returnrout
		ADR	rom_load_readyret



;	TALK routine:
sjl_talk	ora	#$40			;TALK flag
sjl_sendbyatn	sta	z_bsour
		jsr	rom_cbmser_dat_hiz	;DAT line to HiZ
		nop
		nop
		lda	$01
		ora	#%00000100
		sta	$01			;ATN line to Lo

;	Send IEC BYTE: (lwiecs)
sjl_sendbyte	jsr	rom_cbmser_clk_lo	;CLK line to Lo
		jsr	rom_cbmser_dat_hiz	;DAT line to HiZ
		jsr	rom_cbmser_wait1ms	;Wait 1 mSec
		jsr	rom_cbmser_readlines	;Read CBMSER lines: Data -> Cy, Clock: -> N
		bcc	.devpres
		jmp	rom_cbmser_devnotpres	;Set "Device Not Present" flag
.devpres	jsr	rom_cbmser_clk_hiz	;CLK line to HiZ
.waitdata	jsr	rom_cbmser_readlines	;Read CBMSER lines: Data -> Cy, Clock: -> N
		bcc	.waitdata		;Wait until data HiZ
		jsr	rom_cbmser_clk_lo	;CLK line to Lo
		txa
		pha
		ldx	#8			;8 bits to send
.lsendbits	nop
		nop
		nop
		bit	$01
		bmi	.devprescont
		pla
		tax
		jmp	rom_cbmser_timeout	;Set "TimeOut" flag
.devprescont	jsr	rom_cbmser_dat_hiz	;DAT line to HiZ
		ror	z_bsour
		bcs	.dathi
		jsr	rom_cbmser_dat_lo	;DAT line to Lo
.dathi		jsr	rom_cbmser_clk_hiz	;CLK line to HiZ
		lda	$01
		and	#%11111110		;C64: $DF = %11011111	DAT = HiZ
		ora	#%00000010		;C64: $10 = %00010000	CLK = Lo
		sta	$01
		and	#%00000100		;C64: $08 = %00001000	ATN drive?
		beq	.ltwobitsent
		lda	z_bsour
		ror	a
		ror	a
		cpx	#2
		bne	.ltwobitsent
		ldx	#$1e
.lwack1		bit	$01
		bpl	.lwack2
		dex
		bne	.lwack1
		beq	.lcont6

.lwack2		bit	$01
		bpl	.lwack2

.lcont6		ldx	#2
.ltwobitsent	dex
		bne	.lsendbits
		ldx	#$56
.lcont7		dex
		beq	.ltbtimeout
		lda	$01
		bmi	.lcont7
		pla
		tax
		rts

.ltbtimeout	pla
		tax
		jmp	rom_cbmser_timeout	;Set "TimeOut" flag

;	Send Secondary address: (lsendsa)
sjl_sectalk	sta	z_bsour
		jsr	sjl_sendbyte		;Send BYTE
		lda	#%00001001		;C64: $23 = %00100011	Data Lo, ATN / CLK HiZ
		sta	$01
.waitclk	bit	$01
		bvs	.waitclk
		rts

;	UnTalk:
sjl_untalk	lda	$01
		ora	#%00000100		;ATN drive
		sta	$01
		jsr	rom_cbmser_clk_lo
		lda	#$5f			;UNTALK command
		jsr	sjl_sendbyatn		;Send byte with ATN
		jsr	rom_cbmser_atn_hiz	;Release ATN line
		txa
		ldx	#$0a
.ll2		dex
		bne	.ll2
		tax
		jsr	rom_cbmser_clk_hiz	;Release CLK line
		jmp	rom_cbmser_dat_hiz	;Release DAT line

;	IECIN: (lgiecin)
sjl_busin	lda	$01
		cmp	#%01000000		;#$40
		bcc	sjl_busin
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		lda	#%00001000		;ATN/CLK/DAT HiZ
		nop
		nop
		sta	$01		;(0)
		nop
		;nop
		;nop
		lda	$00
		nop
		nop
		nop
		lda	$01		;(15/16)
		lsr
		lsr
		nop
		ora	$01		;(24/26)
		lsr
		lsr
		nop
		eor	$01		;(33/36)
		lsr
		lsr
		eor	#%00001010
		nop
		eor	$01		;(44/48)
		pha
		lda	#%00001001
		bit	$01
		sta	$01
		bvc	.lend1
		bpl	.lerr1
		pla
		lda	#%01000010		;EOI + timeout
		jmp	rom_cbmser_sstatserend	;Set STATUS + serial transfer end
.lerr1		lda	#%01000000		;EOI
		jsr	rom_set_status		;Set Status
.lend1		pla
		clc
		rts

	DEPHASE

loader_code_length = * - loader_code_start

;-----------------------------------------------------------------------------
