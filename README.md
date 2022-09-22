# README

This is an Arduino subroutine to word-wrap lines of text so that line breaks on Adafruit displays happen at whitespace instead of in words. 

Just copy the subroutine from `wordWrap.cpp` into your sketch in the Arduino IDE (or platform io or whatever you use, I suppose).  There's no library to install.   

The routine takes a char array, and will populate a char array that you provide with a "word wrapped" version (ie - `\n` characters in the right places) so the text will word wrap correctly on little OLED/TFT/E-paper displays from Adafruit.  

It relies on functions from the Adafruit GFX library to calculate text bounds so it is doubtful it will work on anything not supported by Adafruit libraries.  I've tested it with the MagTag's E-paper and the Featherwing OLED and it works on them.  Your mileage may vary.  

This is for Arduino code specifically, as CircuitPython can already word wrap.  Works with custom fonts and the GFX default font in the cases I've tested.  See the examples for usage. 

Included are examples: 

`MagTagWrapExample.ino` showing word wrapping on the MagTag 2.9" e-paper display https://www.adafruit.com/product/4800

`SH110XWrapExample.ino` showing word wrapping on the Adafruit 128x64 OLED Featherwing  https://www.adafruit.com/product/4650

`MagTagQuotesWrap.ino` The MagTag 'quotes' demo, but modified to show the quotes with properly word-wrapped text, which was the reason I wrote this routine in the first place.  https://learn.adafruit.com/adafruit-magtag/quotes-example