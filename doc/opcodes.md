# DragonRock Opcodes

## 0x00 NOP / GOTO
No operation

## 0x01 NSTAT <01:msgID>
Change current status message to *msgID*

## 0x02 DISP <01:msgID> <02:clrFlag>
Display *msgID*

## 0x03 WKEY <01:msgID> <02:clrFlag> <03:regNr>
Wait for keypress and display msgID. Stores pressed key in *regNr*

## 0x04 YESNO <01:trueOpcIdx> <02:falseOpcodeIdx>
Wait for 'y' or 'n' keypress
Register 0 -> true on 'yes', otherwise false.
if *y* and *trueOpcIdx*!=0 -> jump to *trueOpcIdx*
if *n* and *falseOpcIdx*!=0 -> jump to *falseOpcodeIdx*

## 0x05 IFREG <01:regNr> <02:regValue> <03:trueOpcIdx> <04:falseOpcIdx>
If register *regNr* contains *regValue*, perform *trueOpcIdx*, else *falseOpcIdx*

## 0x45 IFREG_B <01:regNr> <02:regValue> <03:trueOpcIdx> 
Like IFREG, but **branch** to trueOpcIdx instead of calling it

## 0x06 IFPOS <01:itemId> <02:trueOpcIdx> <02:falseOpcIdx> <03:resultReg> 
If *itemId* is in current party's posession, perform *trueOpcIdx*, else *falseOpcIdx* 
Register #resultReg -> party member who is owner of itemID or 255 for not found

## 0x07 IADD <01:itemId> <02:charIdx> <03:successOpcIdx> <04:failureOpcIdx>
Add *itemId* to character *charIdx* inventory
If *charIdx*==0xff use first free character if posssible
On success, performs <successOpcIdx>; otherwise <failureOpcIdx>
Register 0 -> true on success, otherwise false
Register 1 -> party member who took the item

## 0x87 IADD_V
like IADD; but print '[characterName] took [itemName]' after successful completion

## 0x08 ALTER <01:xpos> <02:ypos> <03:startOpcodeIdx> <04:dungeonItemID>
Alter map at coordinates *xpos*,*ypos* to *startOpcodeIdx* and *dungeonItemID*

## 0x09 REDRAW
Force redraw the dungeon display

## 0x0a ADDC <01:numCoinsL> <02:numCoinsH>
Give numCoins to the party

## 0x8a ADDC_V
Like 'ADDC' but output 'coins taken' message

## 0x0b ADDE <01:numExpL> <02:numExpH>
Give numExp experience to party

## 0x8b ADDE_V <01:numExpL> <02:numExpH>
Like 'ADDE', but output 'exp taken' message

## 0x0c SETREG <01:regNum> <02:regVal>
Set register *regNum* to value *regVal*

## 0x0d CLRENC
Clear encounter list

## 0x0e ADDENC0 <01:mID1> <02:mLvl1> <03:mID2> <04:mLvl2> 
Add up to 2 monsters to encounter row 0

## 0x8e ADDENC1 
Like ADDENC0, but add to encounter row 1

## 0x0f DOENC
Start encounter
On return, register 1==true if won, otherwise (fled enc) false

## 0x1f EXIT <01:mapId> <02:xpos> <03:ypos>
Exits the dungeon module back into wilderness map *mapID* at coords *xpos*, *ypos* 