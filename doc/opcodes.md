# DragonRock Opcodes

## 0x00 NOP
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

## 0x05 IFREG <01:regNr> <02:regValue> <03:trueOpcIdx> <04:falseOpcodeIdx>
If register *regNr* contains *regValue*, perform *trueOpcIdx*, else *falseOpcIdx*

## 0x06 IFPOS <01:itemId> <02:trueOpcIdx> <02:falseOpcIdx> <03:resultReg> 
If *itemId* is in current party's posession, perform *trueOpcIdx*, else *falseOpcIdx* 
Register #resultReg -> party member who is owner of itemID or 255 for not found

## 0x07 IADD <01:itemId> <02:charIdx> <03:successOpcIdx> <04:failureOpcIdx> <05:notifyFlag>
Add *itemId* to character *charIdx* inventory
If *charIdx*==0xff use first free character if posssible
On success, performs <successOpcIdx>; otherwise <failureOpcIdx>
Register 0 -> true on success, otherwise false
Register 1 -> party member who took the item
If <notifyFlag> is set, prints '<characterName> took <itemName>'

## 0x08 ALTER <01:xpos> <02:ypos> <03:dungeonItemID> <04:startOpcodeIDx>
Alter map at coordinates *xpos*,*ypos* to *startOpcodeIdx* and *dungeonItemID*

## 0x09 REDRAW
Force redraw the dungeon display
