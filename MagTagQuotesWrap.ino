// SPDX-FileCopyrightText: 2017 Evandro Copercini
//
// SPDX-License-Identifier: Apache-2.0

/*
  Wifi secure connection example for ESP32
  Running on TLS 1.2 using mbedTLS
  2017 - Evandro Copercini - Apache 2.0 License.
*/
// Originally https://learn.adafruit.com/adafruit-magtag/quotes-example
// Modifications by Steven Cogswell to do word wrapping 
// on the Adafruit MagTag E-paper display September 2022 https://github.com/scogswell/

#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Adafruit_ThinkInk.h"
#include "Adafruit_NeoPixel.h"
#include <Fonts/FreeSans9pt7b.h>

#define DEBUG
#undef DEBUG

const char* ssid     = "Put your ssid here";     // your network SSID (name of wifi network)
const char* password = "put your wifi pasword here"; // your network password

const char*  server = "www.adafruit.com";
const char*  path   = "/api/quotes.php";

WiFiClientSecure client;
ThinkInk_290_Grayscale4_T5 display(EPD_DC, EPD_RESET, EPD_CS, -1, -1);
Adafruit_NeoPixel intneo = Adafruit_NeoPixel(4, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);


void deepSleep() {
  pinMode(NEOPIXEL_POWER, OUTPUT);
  pinMode(SPEAKER_SHUTDOWN, OUTPUT);
  digitalWrite(SPEAKER_SHUTDOWN, LOW); // off
  digitalWrite(NEOPIXEL_POWER, HIGH); // off
  digitalWrite(EPD_RESET, LOW); // off (yes required to save a few mA)
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  esp_sleep_enable_timer_wakeup(60 * 1000000); // 60 seconds
  esp_deep_sleep_start();  
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

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  //while (!Serial) delay(10);

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BUTTON_D, INPUT_PULLUP);
  pinMode(EPD_BUSY, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  
  display.begin(THINKINK_GRAYSCALE4);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  display.clearBuffer();
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  display.setTextColor(EPD_BLACK);
  display.setCursor(10, 30);
  display.print("Connecting to SSID ");
  display.println(ssid);
  display.display();
  
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);

  //client.setCACert(test_root_ca);
  //client.setCertificate(test_client_key); // for client verification
  //client.setPrivateKey(test_client_cert);  // for client verification

// Neopixel power
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, LOW); // on
  intneo.fill(25, 0, 0);
  intneo.show();
  
  Serial.println("\nStarting connection to server...");
  client.setInsecure();
  if (!client.connect(server, 443)) {
    Serial.println("Connection failed!");
    deepSleep();
  }

  intneo.fill(25, 25, 0);
  intneo.show();
  
  Serial.println("Connected to server!");
  // Make a HTTP request:
  client.print("GET "); client.print(path); client.println(" HTTP/1.1");
  client.print("Host: "); client.println(server);
  client.println("Connection: close");
  client.println();

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    deepSleep();
  }

  intneo.fill(0, 25, 0);
  intneo.show();
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  intneo.fill(0, 25, 25);
  intneo.show();
  while (client.peek() != '[') {
    client.read();
  }

  intneo.fill(0, 0, 25);
  intneo.show();
    
  // Allocate the JSON document
  // Use arduinojson.org/v6/assistant to compute the capacity.
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(8) + 200;
  DynamicJsonDocument doc(capacity);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    deepSleep();
  }

  intneo.fill(0, 25, 25);
  intneo.show();
  
  // Extract values
  JsonObject root_0 = doc[0];
  Serial.println(F("Response:"));
  const char* root_0_text = root_0["text"];
  const char* root_0_author = root_0["author"];

  Serial.print("Quote: "); Serial.println(root_0_text);
  Serial.print("Author: "); Serial.println(root_0_author);

  // Buffer to hold the wrapped text
  int NS_SIZE=512; 
  char wrapped[NS_SIZE];


  display.clearBuffer();
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  display.setTextWrap(false);    // Changed to false as we'll do the wrapping ourselves. 
  display.setTextColor(EPD_BLACK);
  display.setCursor(10, 30);
  wrapWords(root_0_text, display.width()-1,wrapped,NS_SIZE);  // New statement 
  display.println(wrapped);    // Changed to wrapped output 
  display.setTextColor(EPD_DARK);
  display.setCursor(40, 120);
  display.println(root_0_author);
  display.display();
  while (!digitalRead(EPD_BUSY)) {
    delay(10);
  }

  while (client.available() > 0)
  {
    //read back one line from the server
    String line = client.readStringUntil('\r');
    Serial.println(line);
  }
  
  // disconnect
  client.stop();
  deepSleep();
}

void loop() {

}
