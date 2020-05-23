# fake-08
A Pico 8 player for 3ds (homebrew)

Usage: currently only supports loading a single cart named "cart.p8". Place cart file next to executable (3dsx or nro) and run from the homebrew menu of your console. 

Building:
Install devkitpro tool chains for 3ds and switch, then call make to make both, or make 3ds or make switch to make one or the other. 

Building tested on windows using devkitpro's msys2 but should work on other platforms as well. 

Games with lots of draw calls will have slowdowns on old 3ds consoles (possibly new ones too?). More optimizations are planned, but keep in mind that Pico 8 lists a raspberry pi 1 with a 700 MHz ARM11 professor as minimum spec, and the old 3ds's CPU is 268 MHz ARM11. Many games should be playable regardless. 

