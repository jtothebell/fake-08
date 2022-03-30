7z d source\carts.zip * -r
7z u source\carts.zip .\carts\*
xxd -i source\carts.zip > source\cartzip.h