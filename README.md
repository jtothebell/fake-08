# fake-08

A Pico 8 player for homebrew consoles. Not related to or supported by Lexaloffle Software. [Latest release](https://github.com/jtothebell/fake-08/releases) includes releases for Nintendo 3DS, Nintendo Switch, Sony PS Vita, Nintendo Wii U, and the Miyoo CFW for bittboy and similar consoles. If you are feeling extra brave, you can also download build aftifacts from the latest CI runs in the [Actions section](https://github.com/jtothebell/fake-08/actions)

## Usage:
Installation will vary by console and executable type. If it is a console with a homebrew menu (Switch, Wii U, 3DS using .3dsx), place the executable file in the directory with other executables. If it is a console with installable hombrew (3ds with .cia, or PS Vita) install executable (VitaShell on Vita or FBI on 3DS).

Pico 8 cart files go in the `p8carts/` directory of your memory card (SD card on 3DS, Switch, and Wii U, memory card at `ux0:/` on Vita). `.p8` text file carts and `.p8.png` image file carts are supported.

Launch FAKE-08 either via the homebrew menu or normal system UI (depending on how you installed). Use left and right to cycle through carts on the SD card. Choose a cart using the `A` (Nintendo consoles) or `X`(Vita) button. To exit the currently running cart, press `Start` or `+` to open the pause menu and select `Exit to Menu`. Press `R` to cycle between rendering sizes. Press `L` and `R` simultaneously to exit the appication. You can also close it via your console's operating system controls (home button etc).

For bittboy and similar consoles, back up `emus/pico8/pico8.elf`  and replace it with the one from the release. Place your cart files in `roms/pico-8/` and use the front end of choice to launch games. Press the menu button to return to the menu (though you can also press start and exit to the FAKE-08 bios menu if you would like).

## Building:
All platforms (except bittboy currently) have automated builds set up via GitHub actions using docker images. You can see how those are set up in the `.github/workflows` directory of the repo.

Building outside of a pre-setup docker container will require a toolchain installation for the platform that you want to build. 

For the Nintendo consoles, install the appropriate toolchain from devkitpro (see https://devkitpro.org/wiki/Getting_Started). Switch and Wii U also require the platform specific SDL2 portlibs to be installed.

Building for the Vita requires Vita SDK (see https://vitasdk.org/) to be installed.

Once you have the appropriate toolchain(s) installed, call `make` followed by the platform name (`3ds`, `switch`, `vita`, or `wiiu`) to build that platform, or just `make` to build them all. `make clean` will clean all files from all platforms.

Building tested on windows using devkitpro's msys2 and Ubuntu (WSL and standalone). Should work on other plaforms as well.

Building for bittboy requires builing your own toolchain first (and will probably only work on unix.). The toolchain is available at https://github.com/bittboy/buildroot/. Clone and build that repo, then recursively copy the contents of `output/host/` to `/opt/miyoo/`. You should then be able to use the `make bittboy` command.

## Acknowledgements
 * Zep/Lexaloffle software for making pico 8. Buy a copy if you can. You won't regret it. https://www.lexaloffle.com/pico-8.php
 * Nintendo Homebrew Community
 * Vita Homebrew Community
 * zepto8 (https://github.com/samhocevar/zepto8) - Probably the best Pico 8 emulator. FAKE-08's audio, tline, emoji conversion, and newer png decompression implementations were ported from zepto8, and other parts were heavily influenced. I also use a slightly modified z8lua (https://github.com/samhocevar/z8lua) for pico 8 specific features.
 * PicoLove (https://github.com/gamax92/picolove) - Noise synthesis ported from this Pico Love, and it was also the basis for my previous project - PicoLovePotion - and where I first learned the basics of Pico 8's API
 * tac08 (https://github.com/0xcafed00d/tac08) - a Pico 8 emulator that I leared a lot from. FAKE-08's sprite rendering and cart parsing were originally based on tac08's implementations
 * LovePotion (https://github.com/TurtleP/LovePotion) - an implementation of Love2d for 3DS and switch that served as the runtime for PicoLovePotion, and a great way to make homebrew games for the 3DS and switch. I also use a modified version of their static Logger implementation

See LICENSE.MD for FAKE-08 license (MIT) as well as licenses of all other software used

## Known Issues:

Latest Pico 8 version v0.2.2 features (sprite fill patterns, text control codes, custom fonts, etc) not implemented yet

Games using `flip()` (like tweetcarts) have intermittent problems exiting back to the menu, and may crash the console. Use with caution.

Sound emulation is not perfect, and the noise implementation is noticably inaccurrate. Most of my sound implementation was ported over from Zepto 8. with the exception of the Noise instrument which was ported from PicoLove. It is not 100% accurate, and some games have noticable clipping/popping.

Performance is not great on Old 3ds systems. Some games may experience slowdowns on the faster consoles as well. More optimizations are probably possible, but keep in mind that Pico 8 lists a raspberry pi 1 with a 700 MHz ARM11 professor as minimum spec, and the old 3DS's CPU is 268 MHz ARM11. Many games should be playable regardless, and hopefully more optimizations can be made.

See [Issues](https://github.com/jtothebell/fake-08/issues) page for more specifics


## Carts

You browse and download carts by using the `SPLORE()` function in Pico 8 (again, if you have $15 to spend, and you are interested in game dev, it is well worth your money). Once you have loaded a cart that you want to try on FAKE-08, type `save {{cartname}}.p8` to save the cart as a text file, then copy that file to your device's SD card.

You can also browse carts on the Pico-8 BBS website, but can only download complete carts in png format. As of pre release v0.0.1.1 Fake-08 should load and play png carts provided they don't use any other unsupported features. You can download p8.png carts from the `Cart` link in the lower left of the game view, and save it into your `p8carts` directory.

If you are trying to play a multi cart game, it should be noted that you must provide all the carts required by the game as FAKE-08 currently does not have cart downloading capabilities. All carts should be placed in the same directory.

## Other Notes

Compatibility is improving, but not perfect. I think many carts _should_ work, but this is still a project in the early stages, and it is my first real foray into C or C++ development and low level game dev in general. I'm mostly doing this project as a fun way to learn.

Feel free to write up any issues you come across, and attach or link to a cart that reproduces the issue. My main goal is to improve compatibility with Pico 8, and then improve speed of carts that are too slow on New 3DS systems.

## Postcard Image From Vita Sprite Credits:
* Rabu Rabu Monster (https://www.lexaloffle.com/bbs/?pid=13897#p) by pedroavelar (No License)
* Rainy Day Friends (https://www.lexaloffle.com/bbs/?pid=16886#p) by electricgryphon (CC4-BY-NC-SA License)
* chrysopoeia (https://www.lexaloffle.com/bbs/?pid=44647#p) by benjamin_soule (No License)
* warehouse panic (https://www.lexaloffle.com/bbs/?pid=46990#p) by benjamin_soule (No License)
* Barp the Balldragon (https://www.lexaloffle.com/bbs/?pid=54395#p) by Saffith (CC4-BY-NC-SA)
* Celeste Classic 2 (https://www.lexaloffle.com/bbs/?pid=celeste_classic_2-5#p) by noel (CC4-BY-NC-SA)
* Solais (https://www.lexaloffle.com/bbs/?pid=d16solais-0#p) by DragonXVI (CC4-BY-NC-SA)
* Demon Castle (https://www.lexaloffle.com/bbs/?pid=demon_castle-1#p) by Mush (CC4-BY-NC-SA)
* Fuz (https://www.lexaloffle.com/bbs/?pid=fuz_v1-1.p8#p) by Jusiv (No License)
* Islander (https://www.lexaloffle.com/bbs/?pid=islander-4.p8#p) by CarsonK (No License)
* Little Necromancer (https://www.lexaloffle.com/bbs/?pid=littlenecromancer-4.p8#p) by Fred_Osterero (CC4-BY-NC-SA)
* Villager (https://www.lexaloffle.com/bbs/?pid=nano_villager-0.p8#p) by partnano (CC4-BY-NC-SA)
* Pico Driller (https://www.lexaloffle.com/bbs/?pid=picodriller-0.p8#p) by johanp (No License)
* pigments (https://www.lexaloffle.com/bbs/?pid=pigments-0.p8#p) by benjamin_soule (CC4-BY-NC-SA)
* Polar Panic (https://www.lexaloffle.com/bbs/?pid=polarpanic-0.p8#p) by johanp (No License)
* Little Architect (https://www.lexaloffle.com/bbs/?pid=ruwukawisa-0.p8#p) by benjamin_soule (No License)
* Scrap Boy (https://www.lexaloffle.com/bbs/?pid=scrap_boy-4.p8#p) by BoneVole (CC4-BY-NC-SA)
* Shelled Shinobi (https://www.lexaloffle.com/bbs/?pid=shelledshinobi1-7.p8#p) by noppa (No License)
* UFO Swamp Odyssey (https://www.lexaloffle.com/bbs/?pid=ufo-0.p8#p) by paranoidcactus (CC4-BY-NC-SA)
