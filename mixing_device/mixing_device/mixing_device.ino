#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // пин 10 = RX, пин 11 = TX

const int NUMBER_OF_BOTTLES = 4; // число емкостей
const int BOTTLE_MAX = 800; // максимальный объем емкости
int bottles_size[ NUMBER_OF_BOTTLES ]; // массив для хранения текущих объемов емкостей

// Pins
const int MODE_BTN_PIN = 3; // кнопка выбора режима работы   
const int LED_PIN = 13; // вывод на светодиод
const int COMPRESSOR_PIN = 4; // вывод к питанию компрессора
const int RELE_PINS[] = { 5, 6, 7, 8 }; // выводы для управления реле
const int SETUP_ANALOG_PIN = A0;  

// Mode
boolean setupMode = true;

// UART consts
const int  GOOD_CONNECTION = 10;
const char FLAG_READ = 'r';
const char FLAG_OK = 'o';
const char FLAG_NOT_OK = 0;
const char FLAG_HAS = 'h';
const char FLAG_FILL = 'f';
const char FLAG_STOP = 's';

// Analog consts
const int MEASURE_TIMES = 10;

int ready_times = 0;

void setup() {
  /*interrupts(); // разрешение прерываний глобально  
  USART_Init(); // инициализация USART */
  mySerial.begin(19200);
  Serial.begin(19200);
  pinMode(10, INPUT); //FIXME
  pinMode(11, OUTPUT); //FIXME
  pinMode(0, INPUT); //FIXME
  pinMode(1, OUTPUT); //FIXME
  
  pinMode(SETUP_ANALOG_PIN, INPUT); 
  pinMode(MODE_BTN_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(COMPRESSOR_PIN, OUTPUT);

  for (int i = 0; i < NUMBER_OF_BOTTLES; i++) {
    pinMode(RELE_PINS[i], OUTPUT);
    bottles_size[i] = 0;
  }
  
  delay(10000);
}

/*void USART_Init() {
  unsigned int baud = 103; // 16*10^6/(16*9600) - 1
  UBRR0H = (unsigned char)(baud >> 8); // установка скорости передачи
  UBRR0L = (unsigned char)baud;
  UCSR0B = (1 << RXCIE0)|(1 << RXEN0)|(1 << TXEN0); // разрешение работы приемника и передатчика, разрешение прерываний от приемника 
  UCSR0C = 3 << UCSZ00; // формат кадра - 8 бит, 1 стоп-бит, нет проверки на четность
}*/

void loop()
{   
  /*if ( mySerial.available() > 0) {
    //Serial.println(mySerial.read());
    char abc = mySerial.read();
    //mySerial.write(abc);
    Serial.println(abc);
  }*/
  
  if (digitalRead(MODE_BTN_PIN) == HIGH) {
   // wait_for_reset_bottle();
  }
  if (digitalRead(MODE_BTN_PIN) == LOW){
    // Рабочий режим
    while ( (mySerial.available() > 0) && (ready_times < GOOD_CONNECTION) ) {
      char command = mySerial.read();
      if (command == FLAG_READ){
        ready_times++;
        Serial.print("Ready_times: ");
        Serial.println(ready_times);
      } else {
        if ( (ready_times != 0) && (command != '\r') && (command != '\n') ) {
          ready_times = 0;
        }
      }
      Serial.print("Got this:");
      Serial.println(command);
    }
    if (ready_times == GOOD_CONNECTION) {
      continue_connection();
      ready_times = 0;
    }
  }
}

void continue_connection() { 
  delay(20);
  mySerial.flush();
  Serial.println("Start answering...");
  for (int i = 0; i < 15; i++) {
    answer_command(FLAG_OK);
  }
}

void answer_command(char command) {
  mySerial.println(command);
}

void wait_for_reset_bottle() {
    unsigned int analogValue = analogRead( SETUP_ANALOG_PIN );
    if (analogValue > 0) {
      for (int i = 1; i < MEASURE_TIMES; i++) {
        analogValue += analogRead(SETUP_ANALOG_PIN);
      }
      analogValue = analogValue / MEASURE_TIMES;
   
      int buttonNumber = get_button_number (analogValue);
      if (buttonNumber > -1) {
        //Serial.println(buttonNumber);
      }
    }
}

void pour_liquid( int bottle_num, int amount ) {
  // calculate the time of working for compressor
  int compressor_time = calc_time( bottle_num, amount );
  
  gain_momentum(); 
    
  // start pour
  digitalWrite(RELE_PINS[ bottle_num ], HIGH);
  digitalWrite(COMPRESSOR_PIN, HIGH);
  delay (compressor_time);
  digitalWrite(COMPRESSOR_PIN, LOW);
  digitalWrite(RELE_PINS[ bottle_num ], LOW);
  return; 
}

int calc_time ( const int bottle_num, const int amount ) {
  int cur_bottle_size = bottles_size[bottle_num];
  int my_amount = amount;
  int need_time = 0;
  if ( cur_bottle_size < (amount + 50) )
    return 0;
  while( my_amount >= 50 ) {
    need_time += linear_calc( cur_bottle_size, 50 ); 
    my_amount -= 50;
    cur_bottle_size -=50;
  }
    need_time += linear_calc( cur_bottle_size, my_amount );
  return need_time;
}

int linear_calc ( const int cur_bottle_size, const int amount ){
 return amount * (3500 - cur_bottle_size) / 50; 
}

int gain_momentum() {
  // time for compressor to gain momentum
  digitalWrite(RELE_PINS[ 3 ], HIGH);
  digitalWrite(COMPRESSOR_PIN, HIGH);
  delay(2000);
  digitalWrite(COMPRESSOR_PIN, LOW);
  digitalWrite(RELE_PINS[ 3 ], LOW); 
}

int get_button_number (unsigned int analog_val) {
  if ( (analog_val >= 200) && (analog_val <= 300) )
    return 0;
  if ( (analog_val >= 460) && (analog_val <= 560) )
    return 1;
  if ( (analog_val >= 720) && (analog_val <= 820) )
    return 2;
  if ( (analog_val >= 923) && (analog_val <= 1023) )
    return 3;
  return -1;
}


