# DragonRock Opcodes

## 0x00 NOP
No operation

## 0x01 NSTAT <01:msgID>
Change current status message to *msgID*

## 0x02 DISP <01:msgID> <02:winColor>
Display *msgID* in a window.

## 0x03 WKEY
Wait for keypress

## 0x04 YESNO <01:trueOpcIdx> <02:falseOpcodeIdx>
Wait for 'y' or 'n' keypress
Register 0 -> true on 'yes', otherwise false.
if *y* and *trueOpcIdx*!=0 -> jump to *trueOpcIdx*
if *n* and *falseOpcIdx*!=0 -> jump to *falseOpcodeIdx*

## 0x05 IFREG <01:regNr> <02:regValue> <03:trueOpcIdx>
If register *regNr* contains *regValue*, jump to *trueOpcIdx*

## 0x06 IFPOS <01:itemId> <02:trueOpcIdx>
If *itemId* is in current party's posession, jump to *trueOpcIdx* 

## 0x07 ADD <01:itemId> <02:charIdx>
Add *itemId* to character *charIdx* inventory
Register 0 -> true on success, otherwise false

## 0x08 ALTER <01:xpos> <02:ypos> <03:startOpcodeIdx> <04:dungeonItemID>
Alter map at coordinates *xpos*,*ypos* to *startOpcodeIdx* and *dungeonItemID*
