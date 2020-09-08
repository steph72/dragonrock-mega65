# dragonrock-mega65
A role playing game for the MEGA65.

I got the idea for "Dragon Rock" upon realizing that there are hardly any native role playing
games for the Commodore TED series of computers (c16, c116, plus/4). Since I always had soft 
spot for the underdogs and also always wanted to implement a classic CRPG, I decided to fill this gap.

But once I had begun implementing DRs advanced features (such as a compiled dungeon description
language and a bytecode interpreter along with it), I realized that the poor old TEDs would have
a hard time processing all the stuff, and the project slowed down and finally was put on hold.

Then along came the amazing MEGA65 re-imagination of Commodore's last 8-bit-machine, and I knew,
the platform to complete "Dragon Rock" was finally there (actually, some 20 years ago, I was the
owner of one of the fabled C65 prototypes, there are even still some demos by yours truly 
floating around on the 'net, so developing for the MEGA65 felt like coming home to me... but that's
a different story).

Gameplay-wise "Dragon Rock" is modelled after "Phantasie" (my favourite crpg series on 8-bit 
computers, because it's simple and challenging at the same time). It's still very much work
in progress and contributions are of course welcome (oh well, who am I kidding ;-)).

Here's some screenshots of what's in there by now:

![Screen1](screenshots/createNew.png =384x)

Character creation in a guild

![Screen2](screenshots/cityNew.png)

City main menu

![Screen3](screenshots/outdoor.png)

Outdoor view

![Screen4](screenshots/dungeon1.jpg)

Dungeon view

![Screen5](screenshots/characterDisplay.jpg)

Character detail in the dungeon. Hubuk has collected the 'rusty key', which...

![Screen6](screenshots/dungeon2.jpg)

...opened this door ;-)

![Screen7](screenshots/mapEditor.png)

Of course, "Dragon Rock" comes with its own map editor...

![Screen8](screenshots/mapCompiler.png)

...and map compiler, which translates a 'mapscript' file into bytecode, which then
gets interpreted by the main binary on the MEGA65.

