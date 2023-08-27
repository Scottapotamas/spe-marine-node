#include <APA102.h>

#define RGB_DATA 16  // Carrier 54, MicroMod RX1/AUDOUT, ESP32 GPIO16
#define RGB_CLK 27  // Carrier/MicroMod D1, ESP32 GPIO27

// Define which pins to use.
const uint8_t dataPin = RGB_DATA;
const uint8_t clockPin = RGB_CLK;

// Create an object for writing to the LED strip.
APA102<dataPin, clockPin> ledStrip;

const uint16_t ledCount = 48;
rgb_color colors[ledCount];

const uint8_t brightness = 2;

// PWM LED settings
const uint8_t pwm_led_pin = 12;  // PWM1 in MicroMod scheme
const uint8_t pwm_led_channel = 0;
const uint8_t pwm_resolution = 8;
const uint16_t pwm_freq = 2000;

void setup()
{
  // Setup PWM output
  ledcSetup( pwm_led_channel, pwm_freq, pwm_resolution );
  ledcAttachPin( pwm_led_pin, pwm_led_channel );
}

/* Converts a color from HSV to RGB.
 * h is hue, as a number between 0 and 360.
 * s is the saturation, as a number between 0 and 255.
 * v is the value, as a number between 0 and 255. */
rgb_color hsvToRgb(uint16_t h, uint8_t s, uint8_t v)
{
    uint8_t f = (h % 60) * 255 / 60;
    uint8_t p = (255 - s) * (uint16_t)v / 255;
    uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t r = 0, g = 0, b = 0;
    switch((h / 60) % 6){
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return rgb_color(r, g, b);
}

uint16_t breatheLED(uint8_t time) {
    // Adjust the period and amplitude for the breathing effect
    double period = 2.0;     // Time for one complete oscillation
    double amplitude = 0.5;  // Brightness amplitude (0 to 1)

    // Calculate the brightness using a sine wave function
    double brightness = amplitude * sin(2.0 * M_PI * time / period) + amplitude;

    // Scale the brightness
    uint16_t scaledBrightness = brightness * 255;

    return scaledBrightness;
}

void loop()
{
  uint8_t time = millis() >> 4;

  for(uint16_t i = 0; i < ledCount; i++)
  {
    uint8_t p = time - i * 8;
    colors[i] = hsvToRgb((uint32_t)p * 359 / 256, 255, 255);
  }

  ledStrip.write(colors, ledCount, brightness);
  
//  uint8_t pwm_val = breatheLED(time);
  ledcWrite( pwm_led_channel, time );



  delay(10);
}
