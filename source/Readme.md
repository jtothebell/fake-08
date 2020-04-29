This is an emulator for Pico 8 games for Nintendo homebrew platforms.

Still in early development, not recommended for others use.

As of now, it should build and run on 3ds, but most carts will crash because the API isn't built out

Initial goal is playability of as many non-tweet cart games as possible

Latter goals include sound support and memory compatibility

Basic Roadmap:
-finish basic graphics api support
(x) implement overloads of existing graphics API
(x) Add camera() support
(x) Add clip() support
(x) Add map(), mget(), mset() support
(x) Add pal() support
(x) Add palt() support
(x) add flip support to sprite drawing


Add support for other non-lua pico functions

(x)Add 30 and 60 fps support

Bugs:
(from jelpi)
button press glyphs are not parsed correctly. (fixed)
// comments should get switched to -- comments (maybe?)
menuitem call should at least be stubbed (done)

from low knight:
0xffff.fffe - hex numbers not converted to numbers properly

thought these were problems, but doesn't appear to be:
%= operator doesn't get replaced. Need to escape somehow? -- this appears to just be an artifact of logging using a format method
while (ta < a-.5) ta += 1 shorthand doesn't get fixed (might be a problem with %=)

(known from docs)
make button press repeats match pico behavior

add stretching options (pixel perfect [current], stretch to fit, stretch and overflow (use bottom screen?))

make flip() work for carts without update

test celeste? some other real world carts (demos?)? alpha release?



add switch, and possibly wii u support



music() and sfx() support
fillp() support
peek() and poke() initial support


Acknowledgements:
Zep for making pico 8
Smea, SciresM, devkitpro devs, Nintendo Homebrew Community
PicoLove
LovePotion
tac08
zepto8


License MIT