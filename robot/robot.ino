#include <Alkoduino.h>
#include  <Wire.h>

#include <SoftwareSerial.h>

#define DEBUG

#define LEFT_SENSOR_PIN  8
#define RIGHT_SENSOR_PIN 9

#define SPEED            135//120//60
#define TURN_SPEED       145//     140//60
#define SLOW_SPEED       125//90//60 
#define BACK_SLOW_SPEED  125//90//60
#define BACK_FAST_SPEED  125//100//60


#define BRAKE_K          4

#define BLACK_THRESHOLD 45

#define STATE_FORWARD    0
#define STATE_RIGHT      1
#define STATE_LEFT       2
#define STATE_STOP       3
#define STATE_HOME       4

#define SPEED_STEP       2

#define FAST_TIME_THRESHOLD     500

#define LEFT_SENSOR P11
#define RIGHT_SENSOR P12

#define TOTAL_STOP_NUMBER 1 // число бутылок - остановок
#define BOTTLE_COUNT 4
#define TIMER_SET_VALUE 4
#define INPUT_TIMEOUT_PARAM 15 // 15*4 c = 60 c (4c - поучено из расечта частоты процессора, делителя таймера и размерности таймера)
#define WAIT_POURING_TIMEOUT 75

#define N_TRIES_WRITE 15
#define N_TRIES_READ 10



//константы - команды для общения с базой по каналу UART
const char UFLAG_READY = 'r';
const char UFLAG_OK = 'o';
const char UFLAG_NOT_OK = 'e';
const char UFLAG_HAS = 'h';
const char UFLAG_FILL = 'f';
const char UFLAG_STOP = 's';
const char UFLAG_COMMAND_END = 'l';
const char UFLAG_OPERATION_END = 'l';
//

int state = STATE_FORWARD;//STATE_HOME;
int currentSpeed = SPEED;
int fastTime = 0;
int prev_right = 0;
int prev_left = 0;

int first_left_state = 0;
int first_right_state = 0;

int bottle_counter = 0;
int is_stop = 0;
int is_home = 0; //предполагаем что изначально находимся на позиции ДОМ

int interrupts_counter = 0; //парметр числа прерываний: T (таймаут) = interrupts_counter * 4с, где 4 с - время до переполнения таймера 1 при частоте 16 000 0000/1024
int is_base_station_timeout = 0;
unsigned char cocktail;

struct liquid {
    unsigned char bottle_number;
    union {
        unsigned char volume_char[2];
	uint16_t volume_uint16_t;
    } volume;
};
typedef liquid RECIPE;


RECIPE recipes[BOTTLE_COUNT]; //число ингридиентов коктейлей ограничено числом бутылок
int ingredient_amount = 4; //0;
int current_ingredient = 0;

int passed_stops = 0;


int is_first_stop_after_start = 1; // флаг того, что программа только начинает работать и она ожидает, что ближайшая остановка  - дом

int gl_speed = 80;
int gl_slow_speed = 70;
int gl_turn_speed = 90;
int gl_back_slow_speed = 70;
int gl_back_fast_speed = 70;


//********************************************************************************************************************************************************************************************************************************************************************

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

int eighth = 0;
int quarter = 2;
int dottedquarter = 3;
int half = 4;
int whole = 8;
// ***************************************************************************************************************************************************************************************************************************************************

void setup() 
{
    //motor_connection(1, 0);
    u_digital_write(L1, LOW);
    u_digital_write(L2, LOW);
    u_digital_write(L3, LOW);
    u_digital_write(L4, LOW);

    Serial.begin(19200);
    
    int prev_left = u_analog_read(LEFT_SENSOR);
    int prev_right = u_analog_read(RIGHT_SENSOR);
    
    analogReference(DEFAULT);
    sei();//разрешаем прерывания глобально
    //USART_init(9600);
    Serial1.begin(19200);
    
    while (!Serial1)
      ; // wait for serial port to connect. Needed for Leonardo only
      
    setuptempo(130);
}


void set_speed (int mls)
{
  switch (mls)
  {
    case 30:
      gl_speed = 100;
      gl_slow_speed = 90;
      gl_turn_speed = 110;
      gl_back_slow_speed = 90;
      gl_back_fast_speed = 90;
      break;
    case 50:
      gl_speed = 115;
      gl_slow_speed = 105;
      gl_turn_speed = 125;
      gl_back_slow_speed = 105;
      gl_back_fast_speed = 105;
      break;
    case 60:
      gl_speed =118;
      gl_slow_speed = 108;
      gl_turn_speed = 128;
      gl_back_slow_speed = 108;
      gl_back_fast_speed = 108;
      break;
    case 80:
      gl_speed = 130;
      gl_slow_speed = 120;
      gl_turn_speed = 140;
      gl_back_slow_speed = 120;
      gl_back_fast_speed = 120;
      break;
    case 90:
      gl_speed = 132;
      gl_slow_speed = 122;
      gl_turn_speed = 142;
      gl_back_slow_speed = 122;
      gl_back_fast_speed = 122;
      break;
    case 100:
      gl_speed = 135;
      gl_slow_speed = 125;
      gl_turn_speed = 145;
      gl_back_slow_speed = 125;
      gl_back_fast_speed = 125;
      break;
    default:
      gl_speed = 80;
      gl_slow_speed = 70;
      gl_turn_speed = 90;
      gl_back_slow_speed = 70;
      gl_back_fast_speed = 70;

  }
}

void runForward()
{
    state = STATE_FORWARD;
    fastTime += 1;
    if (fastTime < FAST_TIME_THRESHOLD) {
        currentSpeed = SLOW_SPEED;
    } else {
        currentSpeed = min(currentSpeed + SPEED_STEP, SPEED);
    }
    u_digital_write(L1, LOW);
    u_digital_write(L2, HIGH);
    u_digital_write(L3, HIGH);
    u_digital_write(L4, LOW);
 
    drive (currentSpeed, currentSpeed);
}

void steerRight() 
{
    state = STATE_RIGHT;
    fastTime = 0;

    //analogWrite(SPEED_RIGHT, 0);
    //nalogWrite(SPEED_LEFT, SPEED);

    //digitalWrite(DIR_LEFT, HIGH);
    //digitalWrite(DIR_RIGHT, HIGH);
    
    u_digital_write(L1, LOW);
    u_digital_write(L2, LOW);
    u_digital_write(L3, LOW);
    u_digital_write(L4, HIGH);
    
    drive (0, TURN_SPEED);
}

void steerLeft() 
{
    state = STATE_LEFT;
    fastTime = 0;

    u_digital_write(L1, HIGH);
    u_digital_write(L2, LOW);
    u_digital_write(L3, LOW);
    u_digital_write(L4, LOW);

    drive(TURN_SPEED, 0);
}


void stepBack(int duration, int state) 
/*
  движение назад
*/
{
    if (!duration) return;

    int leftSpeed = (state == STATE_RIGHT) ? BACK_SLOW_SPEED : BACK_FAST_SPEED;
    int rightSpeed = (state == STATE_LEFT) ? BACK_SLOW_SPEED : BACK_FAST_SPEED;

    //включение 4х светоиодов для отображения направления движения
    u_digital_write(L1, HIGH);
    u_digital_write(L2, HIGH);
    u_digital_write(L3, HIGH);
    u_digital_write(L4, HIGH);    
    drive(rightSpeed, leftSpeed);

    delay(duration);
}

void start_moving()
/*
  начало движения с остановки (включая с домашней позиции)
*/
{
#if defined(DEBUG)
  Serial.println("******************************************* start_moving **********************************************");
#endif

    state = STATE_FORWARD;
    int cur_left, cur_right; // текущее значение на датчиках, левом и правом соответственно
    int ldelta, rdelta; //дельта между исходным (белым!) значением полотона и текущим,  левая и правая соответственно
    boolean left, right; // флаг того, находится ли левое и правое колесо робота на черной полосе (1) или на белой (0)

    while(true)
    {
      runForward(); //начинаем движение вперед
      delay(100);
      cur_left = u_analog_read(LEFT_SENSOR); // считываем текущее значение с левого датчика
      cur_right = u_analog_read(RIGHT_SENSOR); // считываем текущее значение с правого датчика

#if defined(DEBUG)      
      Serial.print("*** cur_left --> "); Serial.println(cur_left);
      Serial.print("first_left_state => "); Serial.println(first_left_state);
      
      Serial.print("*** cur_right --> "); Serial.println(cur_right);
      Serial.print("first_right_state => "); Serial.println(first_right_state);
      Serial.println(" ");
#endif
      
      ldelta = cur_left - first_left_state;
      rdelta = cur_right - first_right_state;
      
      right = !(rdelta < -BLACK_THRESHOLD); // если текущее отклонения значения правого датчика от исходного более, чем на 75 и в отрицательную сторону, значит правое колесо попало на черную линию
      left = !(ldelta < -BLACK_THRESHOLD); // если текущее отклонения значения левого датчика от исходного более, чем на 75 и в отрицательную сторону, значит левое колесо попало на черную линию

      //робот трогается с остановки (черная поперечная полоса), значит как только хотя бы одно колесо попало на белое,
      //то остановка пройдена      
      if (right || left) 
        return;
    }
}

int get_cocktail () 
/*
  по текущей комбинации, введенной с кнопок,  получаем все ингридиенты и их колчиество
  бутылки в масииве рецептов должны быть указаны строго по возрастанию, то есть 0му элементы соотвествует - бутылка с минимальным номером из всех бутылок из коктейля
*/
{
  int is_avaliable_cock = 0; // флаг доступности коктейля (есть он в списке запрограммированных коктейлей или нет)
  //число коктейлей может быть расширено до 8
#if defined(DEBUG)
  Serial.print("get_cocktail => ");
  Serial.println(cocktail);
#endif
  switch (cocktail) {
    case 0x0:
      ingredient_amount = 1; //общее число ингридиентов по данному коктейлю
      recipes[0].volume.volume_uint16_t = 60; //объем жидкости в мл, наливаемый из данной конкретной бутылки
      recipes[0].bottle_number = 1; // число бутылок - от 0 до 3
      is_avaliable_cock = 1;
      break;
      
    case 0x1:
      ingredient_amount = 2; //общее число ингридиентов по данному коктейлю
      recipes[0].volume.volume_uint16_t = 30; //объем жидкости в мл, наливаемый из данной конкретной бутылки
      recipes[0].bottle_number = 0; // число бутылок - от 0 до 3
      
      recipes[1].volume.volume_uint16_t = 30; //объем жидкости в мл, наливаемый из данной конкретной бутылки
      recipes[1].bottle_number = 3; // число бутылок - от 0 до 3
      
      is_avaliable_cock = 1;
      break;
      
    case 0x2:
      ingredient_amount = 2; //общее число ингридиентов по данному коктейлю
      recipes[0].volume.volume_uint16_t = 30; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      recipes[0].bottle_number = 0; // число бутылок - от 0 до 3
      
      recipes[1].volume.volume_uint16_t = 50; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      recipes[1].bottle_number = 2; // число бутылок - от 0 до 3
      
      is_avaliable_cock = 1;
      break;
      
    case 0x3:
      ingredient_amount = 3; //общее число ингридиентов по данному коктейлю
      recipes[0].volume.volume_uint16_t = 30; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      recipes[0].bottle_number = 0; // число бутылок - от 0 до 3
      
      recipes[1].volume.volume_uint16_t = 30; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      recipes[1].bottle_number = 1; // число бутылок - от 0 до 3
      
      recipes[2].volume.volume_uint16_t = 30; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      recipes[2].bottle_number = 3; // число бутылок - от 0 до 3

      is_avaliable_cock = 1;
      break;
      
    case 0x4:
    case 0x5:
    case 0x6:  
    case 0x7:
    default:
      is_avaliable_cock = 0;
  }
  
  if (is_avaliable_cock) return 1;
  return -1;
}

void base_station_timeout ()
/*
  функция обработчик прерываний
*/
{
  is_base_station_timeout = 1;
  Serial.println ("base station timeout");
}

int check_base_station_status_by_serial()
/*
  после получения запроса от пользователя выполняется следующая проверка:
    1. запрос о готовности базы
    2. есть ли ингридиенты на коктейль
*/
{
  delay(100);
  while (Serial1.available() > 0) Serial1.read(); //на всякий случай - для очистки входного буфера 
  is_base_station_timeout = 0;
#if defined(DEBUG)
    Serial.println("check_base_station_status_by_serial");
#endif

  //1. Отправляем запрос о готовности базы, с таймаутом
  char recieved_data; // для хранения данных, полученных из приемника: recieved_data = UDR1 (см. Alkoduino.cpp)
  int is_data = 0; //флаг о получении данных
  Serial1.print(UFLAG_READY); //запрос о готовности базы
  set_timeout(TIMER_SET_VALUE, base_station_timeout); //устанавливаем таймаут равный 4 сек

  /*
  USART_transmit (UFLAG_READY); //72h - ready? - получение данных по уарту
  set_timeout(TIMER_SET_VALUE, base_station_timeout); //устанавливаем таймаут равный 4 сек
  while (USART_recieve(&recieved_data) <= 0 && !is_base_station_timeout) //ожидание получения кода ОК
      ;
  */

  while (!is_base_station_timeout && !is_data) //ожидание получения кода ОК (сброс по таймауту)
  {
      if (Serial1.available() > 0) { //проверка - пришли ли данные
         recieved_data = Serial1.read(); //считывание полученных данных
         
#if defined(DEBUG)
         Serial.print("recieved_data => ");
         Serial.println(recieved_data);
#endif

         if (recieved_data == UFLAG_OK) //проверяем на подтверждение готовности - 'o' = ok
         {
           is_data = 1;
#if defined(DEBUG)
            Serial.println("Is UFLAG_OK");
#endif
         }
         else if (recieved_data == UFLAG_NOT_OK)
         {
#if defined(DEBUG)
            Serial.println("Is UFLAG_NOT_OK");
            Serial.println("processing request terminated");
#endif
            return -1;
         }
      }
  }

  if (is_base_station_timeout) //выход по таймауту
  //база не готова или во время приемо передачи возникла ошибка
  {
#if defined(DEBUG)
       Serial.println("base station is timeouted");
       Serial.println("processing request terminated");
#endif
    return -1;
  }
  
  //2. Вопрос о налаичие жидкости - согласно рецепту, смотрим в какой бутылке сколько жидкости - ПРОВЕРКА выполняется для всех ингридиентов
  int ok = 1; //флаг для индикации состояния проверки, если ok = 0 => произошла ошибка и приемо передача будет преррвана
  is_base_station_timeout = 0;
  for (int i = 0; i < ingredient_amount && ok; i++) {
      Serial1.print (UFLAG_HAS); //68h - has? - префикс команды-запроса о наличии жидкости в определенной бутылке
      Serial1.write (recipes[i].bottle_number); //номер бутылки
      Serial1.write (recipes[i].volume.volume_char[0]); //младший байт мл
      Serial1.write (recipes[i].volume.volume_char[1]); //старший байт мл
      Serial1.print (UFLAG_COMMAND_END); //признак конца команды      
      
#if defined(DEBUG)
      Serial.println ("i => ");
      Serial.println (i);
      Serial.print ("Check ingredient: ");
      Serial.print (UFLAG_HAS); //68h - has? - префикс команды-запроса о наличии жидкости в определенной бутылке
      Serial.print(" - ");
      Serial.print (recipes[i].bottle_number); //номер бутылки
      Serial.print(" - ");
      Serial.print (recipes[i].volume.volume_char[0]); //младший байт мл
      Serial.print(" - ");
      Serial.print (recipes[i].volume.volume_char[1]); //старший байт мл
      Serial.print(" - ");
      Serial.println (UFLAG_COMMAND_END); //признак конца команды    
#endif
      
      /*
          USART_transmit (UFLAG_HAS); //68h - has? - префикс команды-запроса о наличии жидкости в определенной бутылке
          USART_transmit (recipes[i].bottle); //номер бутылки
          USART_transmit (recipes[i].volume.volume_char[0]); //младший байт мл
          USART_transmit (recipes[i].volume.volume_char[1]); //старший байт мл
          USART_transmit (UFLAG_COMMAND_END); //признак конца команды
          set_timeout(TIMER_SET_VALUE, base_station_timeout); //устанавливаем таймаут равный 4 сек
          while (USART_recieve(&recieved_data) <= 0 && !is_base_station_timeout) //ожидание получения кода ОК
            ;
      */
      
      ok = 0; //сбрасываем флаг ok
      is_data = 0; //сбрасываем флаг полученных данных
      set_timeout(TIMER_SET_VALUE, base_station_timeout); //устанавливаем таймаут равный 4 сек
      //while (USART_recieve(&recieved_data) <= 0 && !is_base_station_timeout) //ожидание получения кода ОК
      //  ;
    while (!is_base_station_timeout && !is_data) //ожидание получения кода ОК
    {
        if (Serial1.available() > 0) { //проверка наличия данных в буфере
           recieved_data = Serial1.read(); //чтение полученных данных
  #if defined(DEBUG)
           Serial.print("recieved_data => ");
           Serial.println(recieved_data);
  #endif
           if (recieved_data == UFLAG_OK) //получили подтерждение ('o'= ok)
           {
             is_data = 1;
             ok = 1;
  #if defined(DEBUG)
              Serial.println("Is UFLAG_OK");
  #endif
           }
           else if (recieved_data == UFLAG_NOT_OK)
           {
#if defined(DEBUG)
              Serial.println("Is UFLAG_NOT_OK");
              Serial.println("processing request terminated");
#endif
              return -1;
           }
        }
     }
     if (is_base_station_timeout) //выход по таймауту 
     //база не готова или во время приемо передачи возникла ошибка
     {
#if defined(DEBUG)
          Serial.println("base station is timeouted");
#endif
       ok = 0; //сбрасываем флаг ok
     }
  }
  
  //возвращение результата: -1 - в случае отрицательного результата; 1 -  в случае положительного результата
  if (!ok) return -1;
  return 1;
}

void check_timeout ()
{
  interrupts_counter++; //парметр числа прерываний: T (таймаут) = interrupts_counter * 4с, где 4 с - время до переполнения таймера 1 при частоте 16 000 0000/1024
  if (interrupts_counter < INPUT_TIMEOUT_PARAM) reset_timer_counter(TIMER_SET_VALUE); //если число прерываний меньше требуемого числа устанавливаем новый таймаут по таймеру
}

void run_home()
/*
  функция обработки домашней установки, цикл производится пока установлен флаг is_home
  Робот находится в режиме ожидания ввода кода коктейля - по кнопкам
  Кнопка S1 - кнопка начала ввода и подтверждения
  Кнопки S2, S3, S4 - для ввода кода коктейля.
  После нажатия подтверждения - робот заправшивает наличие жидкостей в бутылках согласно выбранному рецепту
*/
{
#if defined(DEBUG)
  Serial.println("----------------------------------------- run home -----------------------------------------");
#endif
  boolean S1_pushed = 0;
  boolean ok = 0;
  while (is_home) {
    //выключение всех светодиодов
     u_digital_write(L1, LOW);
     u_digital_write(L2, LOW);
     u_digital_write(L3, LOW);
     u_digital_write(L4, LOW);
      while (!S1_pushed) //пока не нажата кнопка S1 - начала ввода - находимся в режиме ожидания
        S1_pushed = u_digital_read(S1); //Считываем положение кнопки, ожидание начала ввода коктейля
      
      interrupts_counter = 0; //парметр числа прерываний: T (таймаут) = interrupts_counter * 4с, где 4 с - время до переполнения таймера 1 при частоте 16 000 0000/1024
      cocktail = 0; //очищаем бит с коктейлем
      delay(500); // устанавливает задержку, чтобы можно было успеть отпустить кнопку
      set_timeout(TIMER_SET_VALUE, check_timeout); //устанавливаем таймаут равный 4 сек  и считаем число перрываний, чтобы задать полный таймаут = 60 с
      while (S1_pushed) { //после нажатия кнопки ввода коктейля (S1), высталвяется флаг S1_pushed, который сбрасывается по таймауту или повторным нажатием кнопки S1 - подтверждение ввода
        u_digital_write(L1, HIGH);
        for (byte i = S2; i <= S4; ++i) // Будем проверять состояние кнопок по очереди
        {
          byte button_number = i - S1; //Определим номер текущей кнопки
          int button_state = u_digital_read(i); //Считаем положение кнопки
          if (button_state) { //если кнопка нажата
            //Подадим считанное значение с кнопки на светодиод с тем же номером            
            u_digital_write(L1 + button_number, button_state); //зажигаем диод, с номер, сответствующим нажатой кнопке (для отображения выбранного коктейля пользователю)
            cocktail |= (1 << (button_state - 1)); //устанваливаем в байте данных о коктейле - бит, соответствующий нажатой кнопке
          }
        }

        if (u_digital_read(S1)) { //кнопка S1-нажата повтроный раз - завершение ввода коктейля
          delay(500);
          ok = 1;
          S1_pushed = 0; //ожидание завершения ввода
          //uint8_t oldSREG = SREG;
          //cli();
          //SREG = oldSREG;
          #if defined(DEBUG)
              Serial.println("S1 pressed one more");
          #endif
        }
        else if (interrupts_counter == INPUT_TIMEOUT_PARAM){
          #if defined(DEBUG)
            Serial.println("INPUT_TIMEOUT_PARAM ERROR");
          #endif
          ok = 0; // ошибка
          S1_pushed = 0; //сбрасываем нажатие
          
          //Мигаем диодами для обозначения - Сообщение об ошибке таймауа
          u_digital_write(L1, LOW);
          u_digital_write(L2, LOW);
          u_digital_write(L3, LOW);
          u_digital_write(L4, LOW);
          delay (100);
          u_digital_write(L1, HIGH);
          u_digital_write(L2, HIGH);
          u_digital_write(L3, HIGH);
          u_digital_write(L4, HIGH);
          delay (1000);
          //Serial.println("TIMEOUT ERORR");
          break;
        }
        u_digital_write(L1, LOW);
      }
      if (ok) {
#if defined(DEBUG)
              Serial.println("is ok entering cocktail");
#endif
        if (get_cocktail () <= 0) ok = 0; // если не удалось получить текущий рецепт, устанавливаем флаг с ошибкой
        else if (check_base_station_status_by_serial() > 0  || true) {
#if defined(DEBUG)
              Serial.println("Base is ready, all ingredients are ready, start moving");
#endif
          passed_stops = 0; //обнуляем счетчик остановок
          current_ingredient = 0; // обнуляем текущую позицию  в массиве ингридиентов recipes
          is_stop = 0; // на всякий случай выставляем в ноль, так как никаких остановок еще нет, только трогаемся с домашней точки
          is_home = 0; //выехали из домашней точки
          start_moving();
        }
        else ok = 0;
      }
  }
}

void base_station_pouring_timeout ()
/*
  callback - функция-обработчик прерывания от таймера при наливании жидкости
*/
{
  interrupts_counter++; //парметр числа прерываний: T (таймаут) = interrupts_counter * 4с, где 4 с - время до переполнения таймера 1 при частоте 16 000 0000/1024
  //if (interrupts_counter < WAIT_POURING_TIMEOUT) reset_timer_counter(TIMER_SET_VALUE); // если чсло прерываний меньше требуемого количества, то устанавливаем новое прерывание
  //else 
  is_base_station_timeout = 1; // таймаут!
}

int run_stop()
/*
  Функция обработки остановки
  1. Посылаем сигнал готовности к наполнению жидкостью для каждого ингредиента последовательно
  2. Получение от базы статуса (таймаут на 5 минут)
  3. если положительный ответ, то продолжаемработу
  4. Ошибка - возвращаемся к домшаней точки без остановок
*/
{
#if defined(DEBUG)
  Serial.println("----------------------------------------- run stop -----------------------------------------");
#endif
      int ok = 1;
      int mls = 0;
      for (int i = 0; i < ingredient_amount && ok; i++)
      {
          is_base_station_timeout = 0; //обнуляем признак таймаута
          interrupts_counter = 0; //параметр числа прерываний: T (таймаут) = interrupts_counter * 4с, где 4 с - время до переполнения таймера 1 при частоте 16 000 0000/1024
          ok = 0; //флаг успешной операции
          //Serial.println ("run_stop");
        
          //1. Посылаем сигнал готовности к наполнению жидкостью
          unsigned char recieved_data; // для хранения данных, полученных из приемника: recieved_data = UDR1 (см. Alkoduino.cpp)
          /*
              USART_transmit (UFLAG_FILL); //66h - fill
              USART_transmit (recipes[current_ingredient].bottle); //номер бутылки
              USART_transmit (recipes[current_ingredient].volume); //мл
              set_timeout(TIMER_SET_VALUE, base_station_pouring_timeout); //устанавливаем таймаут равный 4 сек
              while (USART_recieve(&recieved_data) <= 0 && !is_base_station_timeout) //ожидание получения кода ОК
                ;
          */
          set_timeout(TIMER_SET_VALUE, base_station_pouring_timeout); //устанавливаем таймаут равный 4 сек
          Serial1.print (UFLAG_FILL); //fill? - префикс команды-запроса о начале заполенения резервуара жидкостью из определенной бутылки
          Serial1.write (recipes[i].bottle_number); //номер бутылки
          Serial1.write (recipes[i].volume.volume_char[0]); //младший байт мл
          Serial1.write (recipes[i].volume.volume_char[1]); //старший байт мл
          Serial1.print (UFLAG_COMMAND_END); //признак конца команды
    
#if defined(DEBUG)
          Serial.print ("Filling: ");
          Serial.print (UFLAG_FILL); //68h - has? - префикс команды-запроса о наличии жидкости в определенной бутылке
          Serial.print(" - ");
          Serial.print (recipes[i].bottle_number); //номер бутылки
          Serial.print(" - ");
          Serial.print (recipes[i].volume.volume_char[0]); //младший байт мл
          Serial.print(" - ");
          Serial.print (recipes[i].volume.volume_char[1]); //старший байт мл
          Serial.print(" - ");
          Serial.println (UFLAG_COMMAND_END); //признак конца команды    
#endif
          
          int finished = 0;
          while (!finished && !is_base_station_timeout) { //цикл - пока не сработает таймаут или не будет получено подтверждение
            if (Serial1.available() > 0) {//ожидание поступления данных в буфер
              recieved_data = Serial1.read(); //чтение байта данных из буфера
#if defined(DEBUG)
              Serial.print("received_data => ");
              Serial.println(recieved_data);
#endif
              if (recieved_data == UFLAG_STOP) {
#if defined(DEBUG)
                Serial.print("Is UFLAG_STOP");
#endif
                finished = 1;
                ok = 1;
              }
            }
          }
          if (is_base_station_timeout) { //сработал таймаут
            //база не почему то не наливает или во время приемо передачи возникла ошибка - сброс по таймауту
#if defined(DEBUG)
              Serial.println("Base station is timeouted for fill request");
#endif
            //ok = 0;
            ok = 1;
          }
          mls += recipes[i].volume.volume_uint16_t;
      }
      
      is_stop = 0; // обработка остановки закончилась
      passed_stops++; // проехали еще одну остановку
      
      is_base_station_timeout = 0; //обнуляем на всякий случай
      interrupts_counter = 0; //обнуляем на всякий случай
      
      start_moving(); //начало движения, обработка съезда с поперечной линии
      is_home = 0; //на всякий случай сбрасываем флаг домашней точки
        
      if (!ok) {
        return -1;
      }
#if defined(DEBUG)
      Serial.print("mls => ");
      Serial.println(mls);
#endif     
      set_speed(mls);
      return 1;
}

//*************************************************************************************************************************************************
void check_stop () 
/*
  проверка: является ли данная остановка - домашней точкой, остановкой для налива или останавливаться не требуется
*/
{
#if defined(DEBUG)
  Serial.println("----------------------------------------- check_stop -----------------------------------------");
#endif
    u_digital_write(L1, HIGH);
    u_digital_write(L2, LOW);
    u_digital_write(L3, HIGH);
    u_digital_write(L4, LOW);
      
    if (is_first_stop_after_start > 0) 
    {
      drive(0,0);
      is_home = 1;
      is_stop = 0;
      state = STATE_HOME;
      is_first_stop_after_start = 0;
      
      u_digital_write(L1, LOW);
      u_digital_write(L2, LOW);
      u_digital_write(L3, LOW);
      u_digital_write(L4, LOW);
      delay (100);
      u_digital_write(L1, HIGH);
      u_digital_write(L2, HIGH);
      u_digital_write(L3, HIGH);
      u_digital_write(L4, HIGH);
      delay (1000);
      u_digital_write(L1, LOW);
      u_digital_write(L2, LOW);
      u_digital_write(L3, LOW);
      u_digital_write(L4, LOW);
    }
//  if (current_ingredient >= ingredient_amount) return; //если заполнили всеми ингридиентами из рецепта, то робот следует до домашней точки без остановок
    else if (passed_stops == TOTAL_STOP_NUMBER) { //если проехали все станции, то текущая остановка - дом
      //эта остановка дом
      drive(0,0);
      is_home = 1; //флаг Домашняя остановка устанавливаем
      is_stop = 0; //флаг Текущая остановка сбрасываем
      state = STATE_HOME;

      //Мигаем диодами - как признак, что доехали до домашней точки
      u_digital_write(L1, LOW);
      u_digital_write(L2, LOW);
      u_digital_write(L3, LOW);
      u_digital_write(L4, LOW);
      delay (100);
      u_digital_write(L1, HIGH);
      u_digital_write(L2, HIGH);
      u_digital_write(L3, HIGH);
      u_digital_write(L4, HIGH);
      delay (1000);
      u_digital_write(L1, LOW);
      u_digital_write(L2, LOW);
      u_digital_write(L3, LOW);
      u_digital_write(L4, LOW);
      run_music();
    } 
    else// if (recipes[current_ingredient].bottle_number == passed_stops + 1)//если текущая остановка - остановка для наполнения жидкостью (есть в списке ингридиентов)
    {
      drive(0,0); // останавливаем вращение моторов
      is_stop = 1; // флаг - текущая остановка
      is_home = 0; // флаг Домашняя остновка сбрасываем
    }
}

void check_state () 
/*
  Функция проверки текущего состояния робота при движении.
  Выполняется анализ текущего значения датчиков и принятие решение:
  1. продолжение движения прямо
  2. поворот налево
  3. поворот направо
  4. остановка
*/
{
#if defined(DEBUG)
  Serial.println("----------------------------------------- check_state -----------------------------------------");
#endif
      boolean left, right = 0; // флаг того, находится ли левое и правое колесо робота на черной полосе (1) или на белой (0)
      int cur_left, cur_right; // текущее значение на датчиках, левом и правом соответственно
      int ldelta, rdelta;  //дельта между исходным (белым!) значением полотона и текущим,  левая и правая соответственно
      
      cur_left = u_analog_read(LEFT_SENSOR); // считываем текущее значение с левого датчика
      cur_right = u_analog_read(RIGHT_SENSOR); // считываем текущее значение с правого датчика
      
      
      if( first_left_state == 0) 
      {
        first_left_state = cur_left; // начальная инициализация значений датчиков (ТРЕБОВАНИЕ - изначально поставить робот на белое)
#if defined(DEBUG)
        Serial.print("first_left_state => ");
        Serial.println(first_left_state);
#endif
      }
      if ( first_right_state == 0)
      {
#if defined(DEBUG)
        Serial.print("first_right_state => ");
        Serial.println(first_right_state);
#endif
        first_right_state = cur_right; // начальная инициализация значений датчиков (ТРЕБОВАНИЕ - изначально поставить робот на белое)
      }
      
      ldelta = cur_left - first_left_state;
      rdelta = cur_right - first_right_state;

      right = rdelta < -BLACK_THRESHOLD; // если текущее отклонения значения правого датчика от исходного более, чем на 75 и в отрицательную сторону, значит правое колесо попало на черную линию
      left = ldelta < -BLACK_THRESHOLD; // если текущее отклонения значения левого датчика от исходного более, чем на 75 и в отрицательную сторону, значит левое колесо попало на черную линию
      
      int targetState; // текущее состояние в зависимости от положение колес
      
      if (left == right) {
        if (left && right)
          targetState = STATE_STOP;
        else
          targetState = STATE_FORWARD;
      } else if (left) {
          targetState = STATE_LEFT;
      } else {
          targetState = STATE_RIGHT;
      }
  
      if (state == STATE_FORWARD && targetState != STATE_FORWARD) {
          int brakeTime = (currentSpeed > SLOW_SPEED) ?
              currentSpeed : 0;
          stepBack(brakeTime, targetState);
      }
  
      switch (targetState) {
          case STATE_FORWARD:
#if defined(DEBUG)
                Serial.print("RUN FORWARD => ");
                Serial.print("cur_left: ");
                Serial.print(cur_left);
                Serial.print(", cur_right: ");
                Serial.println(cur_right);
#endif
              runForward(); //движение прямо
              break;
  
          case STATE_RIGHT:
#if defined(DEBUG)
                Serial.print("RUN RIGHT => ");
                Serial.print("cur_left: ");
                Serial.print(cur_left);
                Serial.print(", cur_right: ");
                Serial.println(cur_right);
#endif
              
              steerRight(); //поворот направо
              break;
  
          case STATE_LEFT:
#if defined(DEBUG)
                Serial.print("RUN LEFT => ");
                Serial.print("cur_left: ");
                Serial.print(cur_left);
                Serial.print(", cur_right: ");
                Serial.println(cur_right);
#endif
              steerLeft(); //поворот налево
              break;
          case STATE_STOP:
#if defined(DEBUG)
                  Serial.print("STATE STOP => ");
                  Serial.print("cur_left: ");
                  Serial.print(cur_left);
                  Serial.print(", cur_right: ");
                  Serial.println(cur_right);
#endif
                check_stop(); //проверка остановки - является ли остановка домашней точкой или требуемой остановкой для наполнения жидкостью
              break;
      }
}

ISR(USART_RXC_vect) // interrupt service routine that wraps a user defined function supplied by attachInterrupt
{
    u_digital_write(L1, HIGH);
    u_digital_write(L2, HIGH);
    u_digital_write(L3, HIGH);
    u_digital_write(L4, HIGH);
}


void loop()
/*
  основной цикл, в котором проверяюся флаги:
  1. is_home - является ли остановка домашней станцией
  2. is_stop - является ли остановка требуемой остановкой для наполнения жидкостью
  3. (else) проверка текущего состояния робота при езде, положения колес, выбор дальнейшего поведения
*/
{

   if (is_home && state == STATE_HOME) {//остновка является домашней остановкой
     run_home(); //обработка остановки
   }
   else if (is_stop) { //остановка для наполнения жидкостью
     run_stop(); //обработка остановки
   }   
   if (is_stop == 0 && is_home == 0) //движение робота
   {
     check_state(); //проверка текущего состояния робота
   }
}

//************************************************************************************************************************************************************************************************************************************************************

void playNoteAndLight(int note,int length)
{
  u_digital_write(L1,HIGH);
  u_digital_write(L2,HIGH);
  u_digital_write(L3,HIGH);
  u_digital_write(L4,HIGH);
  tone(BUZZER, note,length);
  delay(length);
  u_digital_write(L1,HIGH);
  u_digital_write(L2,HIGH);
  u_digital_write(L3,HIGH);
  u_digital_write(L4,HIGH);

}

void setuptempo(int starter)
{
    eighth = starter;
    quarter = quarter*starter;
    dottedquarter = dottedquarter*starter;
    half = half*starter;
    whole = whole*starter;
}

void run_music() 
{
    /* jingle bells */
    
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_E4,half);
    
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_E4,half);
    
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_G4,quarter);
    playNoteAndLight(NOTE_C4,dottedquarter);
    playNoteAndLight(NOTE_D4,eighth);
    
    playNoteAndLight(NOTE_E4,whole);
    
    playNoteAndLight(NOTE_F4,quarter);
    playNoteAndLight(NOTE_F4,quarter);
    playNoteAndLight(NOTE_F4,quarter);
    playNoteAndLight(NOTE_F4,quarter);
    
    playNoteAndLight(NOTE_F4,quarter);
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_E4,half);
    
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_D4,quarter);
    playNoteAndLight(NOTE_D4,quarter);
    playNoteAndLight(NOTE_E4,quarter);
    
    playNoteAndLight(NOTE_D4,half);
    playNoteAndLight(NOTE_G4,half);
    
    /* second verse */
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_E4,half);
    
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_E4,half);
    
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_G4,quarter);
    playNoteAndLight(NOTE_C4,dottedquarter);
    playNoteAndLight(NOTE_D4,eighth);
    
    playNoteAndLight(NOTE_E4,whole);
    
    playNoteAndLight(NOTE_F4,quarter);
    playNoteAndLight(NOTE_F4,quarter);
    playNoteAndLight(NOTE_F4,quarter);
    playNoteAndLight(NOTE_F4,quarter);
    
    playNoteAndLight(NOTE_F4,quarter);
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_E4,quarter);
    playNoteAndLight(NOTE_E4,eighth);
    playNoteAndLight(NOTE_E4,eighth);
    
    playNoteAndLight(NOTE_G4,quarter);
    playNoteAndLight(NOTE_G4,quarter);
    playNoteAndLight(NOTE_F4,quarter);
    playNoteAndLight(NOTE_D4,quarter);
    
    playNoteAndLight(NOTE_C4,whole);
}




