// External PWM - White LED

#define PWM_LED 12  // MicroMod PWM1, ESP32 GPIO12

const uint8_t pwm_led_channel = 0;
const uint8_t pwm_resolution = 8;
const uint16_t pwm_freq = 2000;

void led_pwm_setup(void)
{
    ledcSetup( pwm_led_channel, pwm_freq, pwm_resolution );
    ledcAttachPin( PWM_LED, pwm_led_channel );
}

void led_pwm_loop(void)
{
    ledcWrite( pwm_led_channel, millis() >> 4 );
}
