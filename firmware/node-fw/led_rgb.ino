// Internal LED rings

#include <APA102.h>

#define RGB_DATA (16)  // Carrier 54, MicroMod RX1/AUDOUT, ESP32 GPIO16
#define RGB_CLK (27)  // Carrier/MicroMod D1, ESP32 GPIO27
#define APA102_COUNT (48)

APA102<RGB_DATA, RGB_CLK> ledStrip;

rgb_color colors[APA102_COUNT];
const uint8_t brightness = 3;

rgb_color hsv_to_rgb( uint16_t h, uint8_t s, uint8_t v );
rgb_color breathe( uint16_t hue, uint8_t saturation, uint8_t brightMul, uint32_t period, uint32_t systick );

void led_rgb_setup()
{
    // Turn the strip off
    ledStrip.write(colors, APA102_COUNT, 0);
}

void led_rgb_loop()
{
  uint8_t time = millis() >> 4; // RGB wheel time
//  uint32_t time_now = millis();
  
  for( uint16_t i = 0; i < APA102_COUNT; i++ )
  {
//    colors[i] = breathe( 0, 255, 1, 2000, time_now );
//    RGB wheel
    uint8_t p = time - i * 8;
    colors[i] = hsv_to_rgb((uint32_t)p * 359 / 256, 255, 255);
  }

  ledStrip.write(colors, APA102_COUNT, brightness);
}

/* Converts a color from HSV to RGB.
 * h is hue, as a number between 0 and 360.
 * s is the saturation, as a number between 0 and 255.
 * v is the value, as a number between 0 and 255. 
 */
rgb_color hsv_to_rgb( uint16_t h, uint8_t s, uint8_t v )
{
    uint8_t f = (h % 60) * 255 / 60;
    uint8_t p = (255 - s) * (uint16_t)v / 255;
    uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
    
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    
    switch((h / 60) % 6)
    {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    
    return rgb_color(r, g, b);
}

/* Sine-fade a hue/saturation over time 
 * hue is between 0 and 360.
 * saturation is between 0 and 255.
 * brightMul is a >0 scaling factor to convert from the 0-1 intermediate
 */
rgb_color breathe( uint16_t hue, uint8_t saturation, uint8_t brightMul, uint32_t period, uint32_t systick )
{
//  float brightness = 0.5f * (1.0f + sinf(time / 10000));

  uint32_t time_slice = systick % period;      // returns a value between 0 and period-1;
  float angle = (PI * time_slice) / period; // mapping to degrees
  float brightness = 100 * sinf(angle);
  
  return hsv_to_rgb( hue, saturation, brightness * brightMul );
}
