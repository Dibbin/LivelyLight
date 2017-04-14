#include <avr/sleep.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//config
#define LED_COUNT 5
#define LED_PIN 11
#define PIR_PIN 2
#define BRIGHTNESS 255 //0 to 255
unsigned long timeDelay= 2500;
unsigned long animationDuration= 5000;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

//end config

//vars
int i;
int val= 0;
unsigned long now = millis();
unsigned long firstMovement = now;
unsigned long lastMovement = now;
unsigned long animationStartTime = now;
unsigned long elapsed= 0;
unsigned long timeSinceLastMovement = 0; 
unsigned long timeSinceFirstMovement = 0;
unsigned long timeSinceAnimationStart = 0;


float percentComplete=0.0f;
float percentRemaining=0.0f;
bool isFadeIn = true;
bool isAnimating = false;
bool isMotionDetected = false;
bool wasAnimating = false;

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code


  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'
  
  // set the motion sensor input
  pinMode(PIR_PIN, INPUT);
  delay(animationDuration);
}


void loop() {
  readSensor();
  adjustBrightness();

  if (isAnimating) {
    rainbowCycle(20);
    wasAnimating = true;
  } else if (wasAnimating){
    wasAnimating = false;
    strip_push_solid(strip.Color(0,0,0));
  }

  delay(200);
 }


void readSensor() {
  now = millis();

  // read Input, if motion detected, record time
  timeSinceAnimationStart = now - animationStartTime;    
  isAnimating = timeSinceAnimationStart < animationDuration;

  val = digitalRead(PIR_PIN);
  if (val==1) {
    if (!isMotionDetected){
      if (isAnimating) {
        firstMovement = now - (animationDuration - timeSinceAnimationStart);
      } else  {
        firstMovement = now;
        animationStartTime = now;
        timeSinceAnimationStart = 0;
        i++;
      }
      isMotionDetected = true;
    }
      lastMovement = now;
  }

}

void adjustBrightness() {
  timeSinceLastMovement = now - lastMovement;
  timeSinceFirstMovement = now - firstMovement;
    
  // if movement was seen recently turn LED on
  if (timeSinceLastMovement > timeDelay){
    if (isMotionDetected && !isAnimating){
      isMotionDetected = false;
      animationStartTime = now;
      timeSinceAnimationStart = 0;
    }
  }

  if (timeSinceFirstMovement < animationDuration) {
    //fade in animation
    isFadeIn = true;
  } else if (isMotionDetected){
    //solid color
    isFadeIn = false;
  } else {
    //fade out animation
    isFadeIn = false;
  }
  
  if (isFadeIn) {
    percentComplete = ((float)timeSinceFirstMovement) / animationDuration;
    strip.setBrightness(BRIGHTNESS * percentComplete);
  } else {
    if (isMotionDetected){
      percentRemaining = 1.0f; //100%
    } else {
      percentRemaining = 1.0f - min(((float)timeSinceAnimationStart) / animationDuration, 1.0f);
    }
    
    strip.setBrightness(BRIGHTNESS * percentRemaining);
  }
}


void strip_push_solid(uint32_t color) {
    for (uint16_t i=0; i < strip.numPixels(); i=i+1) {
      strip.setPixelColor(i, color);
    }
    strip.show();
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    readSensor();
    adjustBrightness();
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

