uint32_t blink_timestamp = 0;

#define SPE_PSE_VID_DETECTION 13  // Fn board F2, MicroMod PWM0, ESP32 GPIO13
#define SPE_PSE_ENABLE_SOURCE 15  // Fn board F3, MicroMod G0, ESP32 GPIO15

#define SPE_PSE_ENABLE_OUTPUT 25  // Fn board F4, MicroMod G1, ESP32 GPIO25
#define SPE_PSE_FAULT_STATUS  17  // Fn board F6, MicroMod G3, ESP32 GPIO17
#define SPE_PSE_CURRENT_SENSE 34  // Fn board A0, MicroMod A0, ESP32 GPI34/A0

#define SPE_FN_BOARD_INT    14  // Fn board F0, MicroMod D0, ESP32 GPIO14
#define SPE_FN_BOARD_RESET  26  // Fn board F5, MicroMod G2, ESP32 GPIO26
#define SPE_FN_BOARD_CS     5   // Fn board F1, MicroMod SPI_CS, ESP32 CS0/GPIO5

#define RGB_CLK  27  // Carrier/MicroMod D1, ESP32 GPIO27
#define RGB_DATA 16   // Carrier/MicroMod RX1/AUDOUT, ESP32 GPIO16


#define PSE_CURRENT_AMPLIFIER_GAIN (100)
#define PSE_CURRENT_SHUNT_MILIOHMS (50)

#define PSE_VID_SOURCE_WARMUP_MS 7
#define PSE_VID_SOURCE_COOLDOWN_MS 5
#define PSE_VID_TEST_FILTER_COUNTS 10
#define PSE_VID_TEST_TIMEOUT_MS 200
#define PSE_VID_TIMEOUT_HOLDOFF_MS 500
#define PSE_MVFS_THRESHOLD_MA 10
#define PSE_MVFS_DURATION_MS 10

typedef enum {
  PSE_OFF = 0,
  PSE_DETECTION_FAILURE,
  PSE_DETECTION_START,
  PSE_DETECTION_WAIT,
  PSE_DETECTION_CHECK,
  PSE_DETECTION_SUCCESS,
  PSE_ON,
} PODL_DETECTION_STATES;

PODL_DETECTION_STATES pse_state = PSE_OFF;
uint32_t pse_timer;
uint8_t valid_pd_signals = 0;
bool enable_pd_power = false;

float pse_hv_current_mA( void )
{
  // Read the current amplifier
  uint16_t adc_sample = analogRead(SPE_PSE_CURRENT_SENSE);
  
  // Conversion
  float adc_voltage = (adc_sample * 3300) >> 12;
  float shunt_voltage = adc_voltage / PSE_CURRENT_AMPLIFIER_GAIN;
  float current_amps = shunt_voltage / PSE_CURRENT_SHUNT_MILIOHMS;

//  Serial.print("PSE counts: ");
//  Serial.println(adc_sample);
//  Serial.print("    adc_mv: ");
//  Serial.println(adc_voltage);
//  Serial.print("    mA: ");
//  Serial.println(current_amps/1000.0f);
  
  return current_amps/1000.0f;
}

void pse_enable_test_current( bool enable )
{
  digitalWrite( SPE_PSE_ENABLE_SOURCE, enable );
}

bool pse_check_vid_signature( void )
{
  digitalRead(SPE_PSE_VID_DETECTION);
}

void pse_enable_hv_supply( bool enable )
{
  digitalWrite( SPE_PSE_ENABLE_OUTPUT, enable );
}

bool pse_check_hv_fault( void )
{
  // Active low
  return false;//(digitalRead(SPE_PSE_FAULT_STATUS) == LOW);
}

void pse_enable_pd_power( bool active )
{
  enable_pd_power = active;
}

void pse_setup_detection_io(void)
{
    // PoDL Detection Circuit
    pinMode(SPE_PSE_VID_DETECTION, INPUT);
    pinMode(SPE_PSE_ENABLE_SOURCE, OUTPUT);

    // HV (24V) power supply switch IO
    pinMode(SPE_PSE_ENABLE_OUTPUT, OUTPUT);
    pinMode(SPE_PSE_FAULT_STATUS,  INPUT);
    pinMode(SPE_PSE_CURRENT_SENSE, INPUT);
}

// State machine to handle PoDL PSE functionality
// Should be run at >=1kHz
void pse_handle_pd()
{
  switch(pse_state)
  {
    case PSE_OFF:
      pse_enable_hv_supply( false );

      // We want to enable a potential downstream device
      if( enable_pd_power )
      {
        pse_state = PSE_DETECTION_START;
      }
    break;

    // Didn't detect a PD fast enough, etc
    case PSE_DETECTION_FAILURE:
      pse_enable_test_current( false );
      pse_enable_hv_supply( false );
      
      // Wait here for a bit before returning to OFF
      if( millis() - pse_timer >= PSE_VID_TIMEOUT_HOLDOFF_MS )
      {
        pse_timer = millis();
        pse_state = PSE_OFF;
      }
    break;
    
    // Enable the 15mA <5.6V constant current source
    case PSE_DETECTION_START:
      pse_enable_hv_supply( false );
      pse_enable_test_current( true );

      pse_timer = millis();
      pse_state = PSE_DETECTION_WAIT;
    break;

    // Wait for the test current to stabilise, downstream VID circuit to boot
    case PSE_DETECTION_WAIT:
      if( millis() - pse_timer >= PSE_VID_SOURCE_WARMUP_MS )
      {
        pse_timer = millis();
        pse_state = PSE_DETECTION_CHECK;
        valid_pd_signals = 0;
      }
    break;

    // In-spec signature voltage will be confirmed by the comparator
    case PSE_DETECTION_CHECK:
      valid_pd_signals += digitalRead(SPE_PSE_VID_DETECTION);

      // Valid PD signature voltage
      // TODO: Improve debouncing/minimum active time check
      if( valid_pd_signals >= PSE_VID_TEST_FILTER_COUNTS )
      {
        pse_timer = millis();
        pse_state = PSE_DETECTION_SUCCESS;
      }

      // Timeout handling
      if( millis() - pse_timer >= PSE_VID_TEST_TIMEOUT_MS )
      {
        pse_timer = millis();
        pse_state = PSE_DETECTION_FAILURE;
      }
    break;

    // Detected valid PD
    case PSE_DETECTION_SUCCESS:
      pse_enable_test_current( false );

      // Wait for current source to power down
      if( millis() - pse_timer >= PSE_VID_SOURCE_COOLDOWN_MS )
      {
        pse_timer = millis();
        pse_state = PSE_ON;
      }
    break;
    
    case PSE_ON:
      pse_enable_hv_supply( true );

      // Part of the maintain full-voltage signature requires the PD to 
      // draw a minimum current (typically 10mA, but lower is allowed?) every 10ms
      // Lower power devices are meant to pulse their load to achieve this
//      if( pse_hv_current_mA() >= PSE_MVFS_THRESHOLD_MA )
//      {
//        // Reset the timer
//        pse_timer = millis();
//      }
//
//      if( millis() - pse_timer >= PSE_MVFS_DURATION_MS )
//      {
//        pse_timer = millis();
//        pse_state = PSE_DETECTION_FAILURE;
//      }
      
      // Check for faults or user wanting to disable power
      if( pse_check_hv_fault() || !enable_pd_power )
      {
        pse_timer = millis();
        pse_state = PSE_DETECTION_FAILURE;
      }
    break;
    
    default:
      // ???
      pse_state = PSE_DETECTION_FAILURE;
    break;
  }
  
}



void setup()
{
    Serial.begin(115200);
    Serial.println("MicroMod SPE PoDL PSE Fn Board");
    Serial.println("PD Detection and powerup demo");

    pinMode(LED_BUILTIN, OUTPUT);

    pse_setup_detection_io();

    // ADIN1110 IO
    pinMode(SPE_FN_BOARD_INT, INPUT);
//    attachInterrupt(digitalPinToInterrupt(SPE_FN_BOARD_INT), some_fn, CHANGE);
    pinMode(SPE_FN_BOARD_RESET, OUTPUT);
    pinMode(SPE_FN_BOARD_CS, OUTPUT);

}

void loop()
{
    pse_hv_current_mA();
    
    pse_handle_pd();
   
    if( millis() - blink_timestamp >= 250 )
    {
        bool state = !digitalRead(LED_BUILTIN);
        digitalWrite(LED_BUILTIN, state ); 
        blink_timestamp = millis();

        if( millis() >= 5000 )
        {
            // Attempt to power the downstream device
            pse_enable_pd_power( true );
        }
    }
    
    delayMicroseconds(100);
}
