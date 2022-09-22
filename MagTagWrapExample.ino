// Minimal example of word wrapped text on Adafruit MagTag https://learn.adafruit.com/adafruit-magtag/
//
// Steven Cogswell September 2022 https://github.com/scogswell/

#include "Adafruit_ThinkInk.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSansOblique12pt7b.h>

#define DEBUG   // For extra info during processing
#undef DEBUG

// Some meaningless text for demo. 
const char* quote1 = "This quick brown fox has jumped over the lazy dog my dude";
const char* quote2 = "The rain in spain falls mainly on the plain if you ride by train and have some bread";
const char* quote3 = "How many armadillos can you incarcerate between fortnights without amelioration";
const char* quote4 = "I'm pretty sure this will be more than four lines in fact I'd like to get it to be more than five lines because you know a really long line is what people want";
const char* quote5 = "Absolutionist Imperatriumer Ester Absolutionist Imperatriumer Absolutionist Ester Imperatriumer Absolutionist Imperatriumer";

ThinkInk_290_Grayscale4_T5 display(EPD_DC, EPD_RESET, EPD_CS, -1, -1);

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  //while (!Serial) delay(10);
  delay(2000); 

  // This demo doesn't use the buttons but here's the setup for them
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BUTTON_D, INPUT_PULLUP);
  pinMode(EPD_BUSY, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  // Initialize the display on the magtag
  display.begin(THINKINK_GRAYSCALE4);
  display.setRotation(0);
  display.clearBuffer();

  Serial.println("Starting up ");
}


// Routine to word-wrap a char array.
// s         : the char array we want to word wrap
// MAXWIDTH  : pixel position where we have to wrap (e.g. display.width()-1) 
// ns        : the new char array we will make that is word wrapped.  
// NS_LENGTH : the length of allocated space for ns, for bounds checking. 
// 
// Note this routine does not modify s, only ns.
// This relies on having access to the cursor positions 
// display.getCursorX(), displaygetCursorY() and the function 
// display.getTextBounds which should be in the Adafruit GFX library
// for your adafruit display.  
// 
// If your display class variable is not called "display" change it to
// whatever you're using.  
void wrapWords(const char *s, int16_t MAXWIDTH, char ns[], int NS_LENGTH) {
  int16_t x = display.getCursorX();   // Get current cursor position
  int16_t y = display.getCursorY();
  int16_t x1;   // Parameters used for calculating the bounding box 
  int16_t y1; 
  uint16_t w; 
  uint16_t h;  
  char c;      
  int i;
  int last_whitespace; // Position in the buffer we saw the last whitespace character 

#ifdef DEBUG
  Serial.print("nulling array... "); 
#endif
  // Null out the character buffer 
  // If we just fill the buffer with nulls we don't have to worry
  // about remembering to null-terminate the string.  
  for(int j=0; j<NS_LENGTH; j++)
  {
    ns[j]='\0';
  }
  ns[NS_LENGTH-1]='\0';
#ifdef DEBUG
  Serial.println("done");
#endif 
  i=0;
  last_whitespace=0; 
  // Loop through the passed in string, every time there's a space
  // check to see if that would push the bounding box wider than the 
  // display.  If so, go back to the previous whitespace and replace
  // it with a newline \n to force a word wrap. 
  while (c = *s++)
  {
    if (i>NS_LENGTH-1)   // Check if we've written to the end of the buffer, if so just return what's done
      return;  
    if ((char)c==' ')   // Is this the whitespace character
    {
      display.getTextBounds(ns,x,y,&x1,&y1,&w,&h);
#ifdef DEBUG
      // For printing out some information if you want to see this routine in progress.
      Serial.print("Max width is ");
      Serial.println(MAXWIDTH); 
      Serial.print("Whitespace Bounds [");
      Serial.print(ns);
      Serial.print("] are x1=");
      Serial.print(x1);
      Serial.print(" y1=");
      Serial.print(y1);
      Serial.print(" w=");
      Serial.print(w);
      Serial.print(" h=");
      Serial.print(h);
      Serial.println("]");
#endif
      // If w (width of bounding box) + x1 (where the bounding box starts) > our maximum width
      // then go back to the previously found whitespace (ie - the beginning of this word) 
      // and replace it with a newline \n.  
      // Note you use x1 and not x because if we've already wrapped the box x might be different 
      // (ie - now at the left margin) than where you started from the cursor position. 
      if (w+x1 > MAXWIDTH && last_whitespace != 0)
      {  
        ns[last_whitespace]='\n';
      }
      last_whitespace=i;  
    }
    ns[i]=c; // Add char from s into ns. 
    i++;     // increment position.  
  }
  // Edge case: we have to check for the last word to see 
  // if it needs to wrap around
  display.getTextBounds(ns,x,y,&x1,&y1,&w,&h);
  if (w+x1 > MAXWIDTH && last_whitespace != 0)
  {  
    ns[last_whitespace]='\n';
  }
}

// Just a routine to take care of repetitive stuff 
void displayStuff(char *str) {
  display.clearBuffer();
  display.setTextSize(1);
  display.setTextColor(EPD_BLACK);
  display.println(str);
  display.display();      // This is where stuff shows up on the display 
  while (!digitalRead(EPD_BUSY)) {
    delay(10);
  }
}

void loop() {
  int delayTime=1000; 
  int rot=0;   // Screen rotation
  int font_choice=0;  // font demo 
  int x=20;    // Initial Cursor Position 
  int y=30; 
  int NS_SIZE=512;   
  char wrapped[NS_SIZE]; 

  while(font_choice < 3)  // Show three fonts
  {  
    while(rot < 4)   // Over four rotations 
    { 
      display.setTextWrap(false);  // Need this to prevent display from wrapping for us 

      // Pick a font
      if (font_choice == 0) {
        display.setFont(&FreeSans9pt7b);
      } else if (font_choice == 1) {
        display.setFont(&FreeMonoBoldOblique12pt7b);
      } else if (font_choice == 2) {
        display.setFont(&FreeSansOblique12pt7b);
      }

      // Show a bunch of different blocks of text  
      display.setRotation(rot); 
      display.setCursor(x, y);
      wrapWords(quote1, display.width()-1, wrapped, NS_SIZE);
      displayStuff(wrapped);
      delay(delayTime);
      
      display.setCursor(x, y);
      wrapWords(quote2, display.width()-1, wrapped, NS_SIZE);
      displayStuff(wrapped);
      delay(delayTime); 
        
      display.setCursor(x, y);
      wrapWords(quote3, display.width()-1, wrapped, NS_SIZE);
      displayStuff(wrapped);
      delay(delayTime);
      
      display.setCursor(x, y);
      wrapWords(quote4, display.width()-1, wrapped, NS_SIZE);
      displayStuff(wrapped);  
      delay(delayTime); 
      
      display.setCursor(x, y);
      wrapWords(quote5, display.width()-1, wrapped, NS_SIZE);
      displayStuff(wrapped);  
      delay(delayTime); 
  
      // One way to use a String class with the wrap function
      String stn = "This is a string class string not to be confused with a const char string and it's moved with toCharArray()";
      int the_length = stn.length()+1;   // Need room for the \0 at the end
      char the_char_array[the_length];
      stn.toCharArray(the_char_array,the_length); 
      display.setCursor(x, y);
      wrapWords(the_char_array, display.width()-1, wrapped, NS_SIZE);
      displayStuff(wrapped);  
      delay(delayTime); 
  
      // A better way to use the string class with the wrap function 
      String string2 = "This is a string class string not to be confused with a const char string and it's moved with c_str not toCharArray()";
      display.setCursor(x, y);
      wrapWords(string2.c_str(), display.width()-1, wrapped, NS_SIZE);
      displayStuff(wrapped); 
      delay(delayTime); 
  
      rot++;  // Change rotation 
    }
    font_choice++; // next font 
    rot=0; 
  }
}
