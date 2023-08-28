
uint32_t led_pwm_timestamp = 0;
uint32_t led_rgb_timestamp = 0;


void setup()
{
    led_pwm_setup();
    led_rgb_setup();

    led_pwm_timestamp = millis();
    led_rgb_timestamp = millis();
}

void loop()
{
    if( millis() - led_rgb_timestamp >= 20 )
    {
        led_pwm_loop();
    }
 
    if( millis() - led_rgb_timestamp >= 10 )
    {
        led_rgb_loop();
    }
  

}
