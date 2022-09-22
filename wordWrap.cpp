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
// 
// You should have the GFX library word wrap turned off, display.setTextWrap(false)
// For this to work correctly. 
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