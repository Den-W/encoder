#define WITH_LCD 1


#include <ClickEncoder.h>

#if defined (__STM32F1__)  
  // Build for STM32F103CB / STM32F103C8 ARM boards
  #undef WITH_LCD
  #define PIN_LCD PB1     // LCD is on PB1 on MapleMini  
  HardwareTimer timer(1);
#else
  // Build for AVR boards
  #define PIN_LCD PC13    // LCD usually on PC13 on AVR
  #include <TimerOne.h>  
#endif

#ifdef WITH_LCD
#include <LiquidCrystal.h>

#define LCD_RS       8
#define LCD_RW       9
#define LCD_EN      10
#define LCD_D4       4
#define LCD_D5       5
#define LCD_D6       6
#define LCD_D7       7

#define LCD_CHARS   20
#define LCD_LINES    4

LiquidCrystal lcd(LCD_RS, LCD_RW, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
#endif

ClickEncoder *encoder;
int16_t last, value;

void timerIsr() {
  encoder->service();
}

#ifdef WITH_LCD
void displayAccelerationStatus() {
  lcd.setCursor(0, 1);  
  lcd.print("Acceleration ");
  lcd.print(encoder->getAccelerationEnabled() ? "on " : "off");
}
#endif

void setup() {
  
#if defined (__STM32F1__)  
  encoder = new ClickEncoder( PB12, PB13, PB14 );
  pinMode( PIN_LCD, OUTPUT );
  Serial.begin( 115200 );  
  // Wait for USB console to connect
  while( 1 )
  {  if( Serial.isConnected() ) break;
     digitalWrite( PIN_LCD, 1 );
     delay( 50 );
     digitalWrite( PIN_LCD, 0 );
     delay( 300 );          
  }
     
   timer.pause();
   timer.setPeriod( 1000 );
 // Set up an interrupt on channel 1
   timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
   timer.setCompare( TIMER_CH1, 1 );  // Interrupt 1 count after each update
   timer.attachCompare1Interrupt(timerIsr);
   timer.refresh(); // Refresh the timer's count, prescale, and overflow
   timer.resume(); // Start the timer counting
#else
  Serial.begin(9600);
  encoder = new ClickEncoder( A0, A1, A2 );
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
#endif

#ifdef WITH_LCD
  lcd.begin(LCD_CHARS, LCD_LINES);
  lcd.clear();
  displayAccelerationStatus();
#endif
  last = -1;
}

void loop() {  
  value += encoder->getValue();
  
  if (value != last) {
    last = value;
    Serial.print("Encoder Value: ");
    Serial.println(value);
#ifdef WITH_LCD
    lcd.setCursor(0, 0);
    lcd.print("         ");
    lcd.setCursor(0, 0);
    lcd.print(value);
#endif
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    Serial.print("Button: ");
    #define VERBOSECASE(label) case label: Serial.println(#label); break;
    switch (b) {
      VERBOSECASE(ClickEncoder::Pressed);
      VERBOSECASE(ClickEncoder::Held)
      VERBOSECASE(ClickEncoder::Released)
      VERBOSECASE(ClickEncoder::Clicked)
      case ClickEncoder::DoubleClicked:
          Serial.println("ClickEncoder::DoubleClicked");
          encoder->setAccelerationEnabled(!encoder->getAccelerationEnabled());
          Serial.print("  Acceleration is ");
          Serial.println((encoder->getAccelerationEnabled()) ? "enabled" : "disabled");
#ifdef WITH_LCD
          displayAccelerationStatus();
#endif
        break;
    }
  }    
}

