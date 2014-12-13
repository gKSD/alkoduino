#include <Alkoduino.h>
#include  <Wire.h>

#include <SoftwareSerial.h>


#define LEFT_SENSOR_PIN  8
#define RIGHT_SENSOR_PIN 9

#define SPEED            60//120//60
#define TURN_SPEED       70//     140//60
//#define SLOW_SPEED       35//7 
//#define BACK_SLOW_SPEED  30//5
//#define BACK_FAST_SPEED  50//10

#define SLOW_SPEED       60//90//60 
#define BACK_SLOW_SPEED  60//90//60
#define BACK_FAST_SPEED  70//100//60


#define BRAKE_K          4

#define STATE_FORWARD    0
#define STATE_RIGHT      1
#define STATE_LEFT       2
#define STATE_STOP       3
#define STATE_HOME       4

#define SPEED_STEP       2

#define FAST_TIME_THRESHOLD     500

#define RIGHT_THRESHOLD  100
#define LEFT_THRESHOLD  200
#define LEFT_SENSOR P11
#define RIGHT_SENSOR P12

#define TOTAL_STOP_NUMBER 4 // число бутылок - остановок
#define TIMER_SET_VALUE 4
#define INPUT_TIMEOUT_PARAM 15 // 15*4 c = 60 c (4c - поучено из расечта частоты процессора, делителя таймера и размерности таймера)
#define WAIT_POURING_TIMEOUT 75

#define N_TRIES_WRITE 15
#define N_TRIES_READ 10

int state = STATE_HOME;
int currentSpeed = SPEED;
int fastTime = 0;
int prev_right = 0;
int prev_left = 0;

int first_left_state = 0;
int first_right_state = 0;

int buttle_counter = 0;
int is_stop = 0;
int is_home = 1; //предполагаем что изначально находимся на позиции ДОМ

int interrupts_counter = 0; //парметр числа прерываний: T (таймаут) = interrupts_counter * 4с, где 4 с - время до переполнения таймера 1 при частоте 16 000 0000/1024

int is_base_station_timeout = 0;

unsigned char cocktail;

struct RECIPE {
  unsigned char buttle;
  unsigned char volume; // в мл
};

RECIPE current_recipe[TOTAL_STOP_NUMBER]; //число ингридиентов коктейлей ограничено числом бутылок
int ingredient_amount = 0;
int current_ingredient = 0;

int passed_stops = 0;
int ignore_stops = 0;

void setup() 
{
    u_digital_write(L1, LOW);
    u_digital_write(L2, LOW);
    u_digital_write(L3, LOW);
    u_digital_write(L4, LOW);
    
    //uPinMode(19, INPUT);
    
    Serial.begin(19200);
    
    int prev_left = u_analog_read(LEFT_SENSOR);
    int prev_right = u_analog_read(RIGHT_SENSOR);
    
    analogReference(DEFAULT);
    sei();//разрешаем прерывания глобально
    //USART_init(9600);
    Serial1.begin(19200);
    //while (!mySerial) {
    //;   // wait for serial port to connect. Needed for Leonardo only
    //}
    while (!Serial1)
      ; // wait for serial port to connect. Needed for Leonardo only
      
    //delay (200000);
}


void base_station_timeout ()
/*
  функция обработчик прерываний
*/
{
  is_base_station_timeout = 1;
  //Serial.println ("base station timeout");
}

void base_station_timeout_test1 ()
/*
  функция обработчик прерываний
*/
{
  is_base_station_timeout = 1;
  //Serial.println ("base station timeout");
}


void send_byte(char send_byte)
{
#if defined(N_TRIES_WRITE)
  for (int i = 0; i < N_TRIES_WRITE; i++)
  {
    Serial1.println('r');
  }
#else
  Serial1.print(send_byte);
#endif
}

void test_function1 ()
{
  //delay(5000);
  //send_byte('r');
  //is_base_station_timeout = 0;
  //set_timeout(TIMER_SET_VALUE, base_station_timeout); //устанавливаем таймаут равный 4 сек
  /*
     while ( !Serial1.available()) Serial.println("waiting");;
      
      while ( Serial1.available())
      {
        //char cc = Serial1.read();
        //char ss[100];
        //sprintf(ss, "LINE: %c",cc);

        char cc = Serial1.read();
        Serial.print(cc);
        if( cc == 'o')
          Serial.println("OK");
        else Serial.print ("Dermo");
        //Serial1.flush();
      }
    */  
    
  //delay (5000);



  int S1_pushed = 0;

  u_digital_write(L1, LOW);
  u_digital_write(L2, LOW);
  u_digital_write(L3, LOW);
  u_digital_write(L4, LOW);

  do
  {
    S1_pushed = u_digital_read(S1); //Считаем положение кнопки, ожидание начала ввода коктейля
  }
  while(S1_pushed == 0);
  delay(300);
  u_digital_write(L4, HIGH);

  send_byte('r');
  //Serial1.flush();
  
  S1_pushed = 0;  

  while (!(Serial1.available() > 0)) ;
  
  int read_bytes = 0;
  
  while (Serial1.available() > 0) {
    char cc = Serial1.read();
    Serial.print("Read: ");
    Serial.println(cc);
    if (cc == 'o') 
    {
      Serial.println("Is 'o'");
      //read_bytes++;
    }
          read_bytes++;
    //if (read_bytes == N_TRIES_READ) break;
  }
  delay(20);
  Serial1.flush();

    Serial.print("ALLLLLLLLL => ");
    Serial.println(read_bytes);
  
  u_digital_write(L1, HIGH);
  u_digital_write(L2, HIGH);
  u_digital_write(L3, HIGH);
  u_digital_write(L4, HIGH);
  
  //delay(50);
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

    //analogWrite(SPEED_LEFT, 0);
    //analogWrite(SPEED_RIGHT, SPEED);

    //digitalWrite(DIR_LEFT, HIGH);
    //digitalWrite(DIR_RIGHT, HIGH);
  
  
    drive(TURN_SPEED, 0);
}


void stepBack(int duration, int state) {
    if (!duration)
        return;

    //Serial.print ("STEP BACK");
    int leftSpeed = (state == STATE_RIGHT) ? BACK_SLOW_SPEED : BACK_FAST_SPEED;
    int rightSpeed = (state == STATE_LEFT) ? BACK_SLOW_SPEED : BACK_FAST_SPEED;

    //analogWrite(SPEED_LEFT, leftSpeed);
    //analogWrite(SPEED_RIGHT, rightSpeed);

    //СЂРµРІРµСЂСЃ РєРѕР»РµСЃ
    //digitalWrite(DIR_RIGHT, LOW);
    //digitalWrite(DIR_LEFT, LOW);

    u_digital_write(L1, HIGH);
    u_digital_write(L2, HIGH);
    u_digital_write(L3, HIGH);
    u_digital_write(L4, HIGH);    
    drive(rightSpeed, leftSpeed);

    delay(duration);
}

void run_drinks()
{}


void start_moving()
/*
  начало движения с остановки (включая с домашней позиции)
*/
{
    int cur_left, cur_right; // текущее значение на датчиках, левом и правом соответственно
    int ldelta, rdelta; //дельта между исходным (белым!) значением полотона и текущим,  левая и правая соответственно
    boolean left, right; // флаг того, находится ли левое и правое колесо робота на черной полосе (1) или на белой (0)

    while(true)
    {
      runForward(); //начинаем движение вперед
      cur_left = u_analog_read(LEFT_SENSOR); // считываем текущее значение с левого датчика
      cur_right = u_analog_read(RIGHT_SENSOR); // считываем текущее значение с правого датчика
      
      ldelta = cur_left - first_left_state;
      rdelta = cur_right - first_right_state;
      
      right = rdelta < -75; // если текущее отклонения значения правого датчика от исходного более, чем на 75 и в отрицательную сторону, значит правое колесо попало на черную линию
      left = ldelta < -75; // если текущее отклонения значения левого датчика от исходного более, чем на 75 и в отрицательную сторону, значит левое колесо попало на черную линию

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
  switch (cocktail) {
    case 0x0:
      ingredient_amount = 1; //общее число ингридиентов по данному коктейлю
      current_recipe[0].volume = 58; //объем жидкости в мл, наливаемый из данной конкретной бутылки
      current_recipe[0].buttle = 2; // число бутылок - от 1 до 4
      is_avaliable_cock = 1;
      break;
      
    case 0x1:
      ingredient_amount = 2; //общее число ингридиентов по данному коктейлю
      current_recipe[0].volume = 30; //объем жидкости в мл, наливаемый из данной конкретной бутылки
      current_recipe[0].buttle = 1; // число бутылок - от 1 до 4
      
      current_recipe[1].volume = 30; //объем жидкости в мл, наливаемый из данной конкретной бутылки
      current_recipe[1].buttle = 4; // число бутылок - от 1 до 4
      
      is_avaliable_cock = 1;
      break;
      
    case 0x2:
      ingredient_amount = 3; //общее число ингридиентов по данному коктейлю
      current_recipe[0].volume = 10; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      current_recipe[0].buttle = 1; // число бутылок - от 1 до 4
      
      current_recipe[1].volume = 35; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      current_recipe[1].buttle = 2; // число бутылок - от 1 до 4
      
      current_recipe[2].volume = 20; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      current_recipe[2].buttle = 3; // число бутылок - от 1 до 4
      is_avaliable_cock = 1;
      break;
      
    case 0x3:
      ingredient_amount = 4; //общее число ингридиентов по данному коктейлю
      current_recipe[0].volume = 10; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      current_recipe[0].buttle = 1; // число бутылок - от 1 до 4
      
      current_recipe[1].volume = 10; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      current_recipe[1].buttle = 2; // число бутылок - от 1 до 4
      
      current_recipe[2].volume = 10; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      current_recipe[2].buttle = 3; // число бутылок - от 1 до 4
      
      current_recipe[3].volume = 15; //объем жидкости в мл, наливаемый из данной конкретной бутылк
      current_recipe[3].buttle = 4; // число бутылок - от 1 до 4
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


int check_base_station_status()
/*
  после получения запроса от пользователя выполняется следующая проверка:
    1. запрос о готовности базы
    2. есть ли ингридиенты на коктейль
*/
{
  is_base_station_timeout = 0;
  //Serial.println ("check_base_station_status");

  //1. Отправляем запрос о готовности базы, с таймаутом
  unsigned char recieved_data; // для хранения данных, полученных из приемника: recieved_data = UDR1 (см. Alkoduino.cpp)
  USART_transmit ('r'); //72h - ready? - получение данных по уарту
  set_timeout(TIMER_SET_VALUE, base_station_timeout); //устанавливаем таймаут равный 4 сек
  while (USART_recieve(&recieved_data) <= 0 && !is_base_station_timeout) //ожидание получения кода ОК
    ;
  if (is_base_station_timeout) {
    //база не готова или во время приемо передачи возникла ошибка
    return -1;
  }
  else {
    if (recieved_data != 0x67) { // Код ОК = 0x67 = 'o'
      return -1; //команда не правильная
    }
  }

  //2. Вопрос о налаичие жидкости - согласно рецепту, смотрим в какой бутылке сколько жидкости - ПРОВЕРКА выполняется для всех ингридиентов
  int ok = 1; //флаг для индикации состояния проверки, если ok = 0 => произошла ошибка и приемо передача будет преррвана
  is_base_station_timeout = 0;
  for (int i = 0; i < TOTAL_STOP_NUMBER && ok; i++) {
      USART_transmit ('h'); //68h - has?
      USART_transmit (current_recipe[i].buttle); //номер бутылки
      USART_transmit (current_recipe[i].volume); //мл
      set_timeout(TIMER_SET_VALUE, base_station_timeout); //устанавливаем таймаут равный 4 сек
      while (USART_recieve(&recieved_data) <= 0 && !is_base_station_timeout) //ожидание получения кода ОК
        ;
      if (is_base_station_timeout)
        ok = 0; //база не готова или во время приемо передачи возникла ошибка
      else {
        if (recieved_data != 0x67) // Код ОК = 0x67 = 'o'
          ok = 0; // полученная команда не верная
      }
  }
  if (!ok) return -1;
  return 1;
}

int check_base_station_status_fake()
/*
  после получения запроса от пользователя выполняется следующая проверка:
    1. запрос о готовности базы
    2. есть ли ингридиенты на коктейль
*/
{
/*  int rr = 0;
  is_base_station_timeout = 0;
  //Serial.println ("check_base_station_status");

  //1. Отправляем запрос о готовности базы, с таймаутом
  unsigned char recieved_data; // для хранения данных, полученных из приемника: recieved_data = UDR1 (см. Alkoduino.cpp)
  //Serial.println('r'); //72h - ready? - получение данных по уарту
  set_timeout(TIMER_SET_VALUE, base_station_timeout); //устанавливаем таймаут равный 4 сек
  while (!rr && !is_base_station_timeout) {
    while (//Serial.available() > 0 ) {
      recieved_data = //Serial.read();
      rr = 1;
    }
  }
  if (is_base_station_timeout) {
    //база не готова или во время приемо передачи возникла ошибка
    return -1;
  }
  else {
    if (recieved_data != 0x67) { // Код ОК = 0x67 = 'o'
      return -1; //команда не правильная
    }
  }

  //2. Вопрос о налаичие жидкости - согласно рецепту, смотрим в какой бутылке сколько жидкости - ПРОВЕРКА выполняется для всех ингридиентов
  int ok = 1; //флаг для индикации состояния проверки, если ok = 0 => произошла ошибка и приемо передача будет преррвана
  is_base_station_timeout = 0;
  for (int i = 0; i < TOTAL_STOP_NUMBER && ok; i++) {
      //Serial.println('h'); //68h - has?
      //Serial.println(current_recipe[i].buttle); //номер бутылки
      //Serial.println(current_recipe[i].volume); //мл
      set_timeout(TIMER_SET_VALUE, base_station_timeout); //устанавливаем таймаут равный 4 сек
      rr = 0;
      while (!rr && !is_base_station_timeout) {
        while (//Serial.available() > 0) //ожидание получения кода ОК
        {recieved_data = //Serial.read(); rr = 1;}
      }
      if (is_base_station_timeout)
        ok = 0; //база не готова или во время приемо передачи возникла ошибка
      else {
        if (recieved_data != 0x67) // Код ОК = 0x67 = 'o'
          ok = 0; // полученная команда не верная
      }
  }
  if (!ok) return -1;
  return 1;
  */
}

void check_timeout ()
{
  interrupts_counter++; //парметр числа прерываний: T (таймаут) = interrupts_counter * 4с, где 4 с - время до переполнения таймера 1 при частоте 16 000 0000/1024
  if (interrupts_counter < INPUT_TIMEOUT_PARAM) reset_timer_counter(TIMER_SET_VALUE); //если число прерываний меньше требуемого числа устанавливаем новый таймаут по таймеру
  ////Serial.println("TIMERRRRRRRRRRRRRRRRRRRR");
}

void run_home()
{
  boolean S1_pushed = 0;
  boolean ok = 0;
  while (is_home) {
     u_digital_write(L1, LOW);
     u_digital_write(L2, LOW);
     u_digital_write(L3, LOW);
     u_digital_write(L4, LOW);
      while (!S1_pushed) 
        S1_pushed = u_digital_read(S1); //Считаем положение кнопки, ожидание начала ввода коктейля
      
      interrupts_counter = 0; //парметр числа прерываний: T (таймаут) = interrupts_counter * 4с, где 4 с - время до переполнения таймера 1 при частоте 16 000 0000/1024
      cocktail = 0; //очищаем бит с коктейлем
      delay(500); // устанавливает задержку, чтобы можно было успеть отпустить кнопку
      set_timeout(TIMER_SET_VALUE, check_timeout); //устанавливаем таймаут равный 4 сек  и считаем число перрываний, чтобы задать полный таймаут = 60 с
      while (S1_pushed) { //после нажатия кнопки ввода коктейля (S1), высталвяется флаг S1_pushed, который сбрасывается по таймауту или повторным нажатием кнопки S1 - подтверждение ввода
        //Serial.println (interrupts_counter);
        u_digital_write(L1, HIGH);
        for (byte i = S2; i <= S4; ++i) // Будем проверять состояние кнопок по очереди
        {
          byte button_number = i - S1; //Определим номер текущей кнопки
          int button_state = u_digital_read(i); //Считаем положение кнопки
          ////Serial.println(interrupts_counter);     
          if (button_state) { //если кнопка нажата
            //Подадим считанное значение с кнопки на светодиод с тем же номером            
            u_digital_write(L1 + button_number, button_state); //зажигаем диод, с номер, сответствующим нажатой кнопке (для отображения выбранного коктейля пользователю)
            cocktail |= (1 << (button_state - 1)); //устанваливаем в байте данных о коктейле - бит, соответствующий нажатой кнопке
          }
        }
        if (u_digital_read(S1)) { //кнопка S1-нажата повтроный раз - завершение ввода коктейля
          ok = 1;
          S1_pushed = 0; //ожидание завершения ввода
          //uint8_t oldSREG = SREG;
          //cli();
          //SREG = oldSREG;
        }
        else if (interrupts_counter == INPUT_TIMEOUT_PARAM){
          //Serial.println ("ERROR");
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
        //Serial.println ("state is ok");
        if (get_cocktail () <= 0) ok = 0; // если не удалось олучить текущий рецепт, устанавливаем флаг с ошибкой
        else if (check_base_station_status_fake() > 0) {
          //Serial.println ("OK");
          passed_stops = 0; //обнуляем счетчик остановок
          current_ingredient = 0; // обнуляем текущую позицию  в массиве ингридиентов current_recipe
          is_stop = 0; // на всякий случай выставляем в ноль, так как никаких остановок еще нет, только трогаемся с домашней точки
          start_moving();
          is_home = 0; //выехали из домашней точки
          ignore_stops = 0; //пока ошибок нет, едем с всеми остановками
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
  if (interrupts_counter < WAIT_POURING_TIMEOUT) reset_timer_counter(TIMER_SET_VALUE); // если чсло прерываний меньше требуемого количества, то устанавливаем новое прерывание
  else is_base_station_timeout = 0; // таймаут!
}

int run_stop()
/*
  Функция обработки остановки
  1. Посылаем сигнал готовности к наполнению жидкостью
  2. Получение то базы статуса (таймаут на 5 минут)
  3. если положительный ответ, то продолжаем движение к следующей остановке
  4. Ошибка - возвращаемся к домшаней точки без остановок
*/
{
      is_base_station_timeout = 0; //обнуляем признак таймаута
      interrupts_counter = 0; //парметр числа прерываний: T (таймаут) = interrupts_counter * 4с, где 4 с - время до переполнения таймера 1 при частоте 16 000 0000/1024
      int ok = 1; //флаг успешной операции
      //Serial.println ("run_stop");
    
      //1. Посылаем сигнал готовности к наполнению жидкостью
      unsigned char recieved_data; // для хранения данных, полученных из приемника: recieved_data = UDR1 (см. Alkoduino.cpp)
      USART_transmit ('f'); //66h - fill
      USART_transmit (current_recipe[current_ingredient].buttle); //номер бутылки
      USART_transmit (current_recipe[current_ingredient].volume); //мл
      set_timeout(TIMER_SET_VALUE, base_station_pouring_timeout); //устанавливаем таймаут равный 4 сек
      while (USART_recieve(&recieved_data) <= 0 && !is_base_station_timeout) //ожидание получения кода ОК
        ;
      if (is_base_station_timeout) {
        //база не почему то не наливает или во время приемо передачи возникла ошибка
        ok = 0;
      }
      else if (recieved_data != 0x67) { // Код ОК = 0x67 = 'o'
          ok = 0; //команда не правильная
        }
      else ok = 1; //успешное завршение операции

    current_ingredient++; //обработали ингридиент, едем к следующему
    is_stop = 0; // обработка остановки закончилась
    passed_stops++; // проехали еще одну остановку
    
    is_base_station_timeout = 0; //обнуляем на всякий случай
    interrupts_counter = 0; //обнуляем на всякий случай
      
    if (!ok) {
      ignore_stops = 1; // в случае возникновения ошибки - возвращаемся на домашнюю точек без остановок.
      return -1;
    }
    return 1;
}

//*************************************************************************************************************************************************
void check_stop () 
/*
  проверка: является ли данная остановка - домашней точкой, нужной остановкой или останавливаться не требуется
*/
{
    if (ignore_stops > 0) return; //если была ошибка и включен флаг ignore_stops - то робот следует до домашней точек без остановок
    if (passed_stops == TOTAL_STOP_NUMBER) { //если проехали все станции, то текущая остановка - дом
      //это остановка дом
      drive(0,0);
      is_home = 1; //флаг Домашняя остановка устанавливаем
      is_stop = 0; //флаг Текущая остановка сбрасываем
      state = STATE_HOME;
      
      //Мигаем диодами - как признак что доехали съдо домашней точки
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
    else if (current_recipe[current_ingredient].buttle == passed_stops + 1){ //если текущая остановка - остановка для наполнения жидкостью (есть в списке ингридиентов)
      //наливаем
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
      boolean left, right = 0; // флаг того, находится ли левое и правое колесо робота на черной полосе (1) или на белой (0)
      int cur_left, cur_right; // текущее значение на датчиках, левом и правом соответственно
      int ldelta, rdelta;  //дельта между исходным (белым!) значением полотона и текущим,  левая и правая соответственно
      
      cur_left = u_analog_read(LEFT_SENSOR); // считываем текущее значение с левого датчика
      cur_right = u_analog_read(RIGHT_SENSOR); // считываем текущее значение с правого датчика
      
      
      if( first_left_state == 0) first_left_state = cur_left; // начальная инициализация значений датчиков (ТРЕБОВАНИЕ - изначально поставить робот на белое)
      if ( first_right_state == 0) first_right_state = cur_right; // начальная инициализация значений датчиков (ТРЕБОВАНИЕ - изначально поставить робот на белое)
      
      ldelta = cur_left - first_left_state;
      rdelta = cur_right - first_right_state;

      right = rdelta < -75; // если текущее отклонения значения правого датчика от исходного более, чем на 75 и в отрицательную сторону, значит правое колесо попало на черную линию
      left = ldelta < -75; // если текущее отклонения значения левого датчика от исходного более, чем на 75 и в отрицательную сторону, значит левое колесо попало на черную линию
      
     /* if (left == 0 && right == 0) {
        first_left_state = cur_left;
        first_right_state = cur_right;
      }*/ 

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
              //Serial.println("*** FORWARD");
              //Serial.println(cur_left);
              //Serial.println(cur_right);
              runForward(); //движение прямо
              break;
  
          case STATE_RIGHT:
                    //Serial.println("*** RIGHT");
                    //Serial.println(cur_right);
                    //Serial.println(first_right_state);
              steerRight(); //поворот направо
              break;
  
          case STATE_LEFT:
                    //Serial.println("*** LEFT");
                    //Serial.println(cur_left);
                    //Serial.println(first_left_state);
              steerLeft(); //поворот налево
              break;
          case STATE_STOP:
                //Serial.println("*** STOPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP *************************** ");
                check_stop(); //проверка остановки - является ли остановка домашней точкой или требуемой остановкой для наполнения жидкостью
              break;
      }
}

void test_function() {
     boolean left, right = 0;

    int cur_left, cur_right;
    int ldelta, rdelta;

    //drive(60, 60);
    while (true)
    {
      delay (1000);
      
      cur_left = u_analog_read(LEFT_SENSOR);
      cur_right = u_analog_read(RIGHT_SENSOR);
      
      
      if( first_left_state == 0) first_left_state = cur_left;
      if ( first_right_state == 0) first_right_state = cur_right;
      
      ldelta = cur_left - first_left_state; //prev_left;
      rdelta = cur_right - first_right_state; //prev_right;

      //ldelta = cur_left - prev_left;
      //rdelta = cur_right - prev_right;
      
      right = abs(rdelta) > 75 && rdelta < 0;
      left = abs(ldelta) > 75 && ldelta < 0;
      
      if (left == 0 && right == 0) {
        first_left_state = cur_left;
        first_right_state = cur_right;
      }
      
      //Serial.println("*** left -->");
      //Serial.println(cur_left);
      //Serial.println(first_left_state);
      //Serial.println(ldelta);
      //Serial.println(left);
      //Serial.println("*** right -->");
      //Serial.println(cur_right);
      //Serial.println(first_right_state);
      //Serial.println(rdelta);
      //Serial.println(right);
      
      if (!left) prev_left = cur_left;
      if (!right) prev_right = cur_right;
      
      ////Serial.println("left -->");
      ////Serial.println(left);
      ////Serial.println("right -->");
      ////Serial.println(right);
      if (left)
      {
        u_digital_write(L1, HIGH);
        u_digital_write(L2, LOW);
        u_digital_write(L3, LOW);
        u_digital_write(L4, LOW);
      }
      if (right)
      {
        u_digital_write(L1, LOW);
        u_digital_write(L2, LOW);
        u_digital_write(L3, LOW);
        u_digital_write(L4, HIGH);
      }
      if (left == right) {
            u_digital_write(L1, LOW);
            u_digital_write(L2, HIGH);
            u_digital_write(L3, HIGH);
            u_digital_write(L4, LOW);
      }
      delay (3000);
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
  boolean debug = true; 
  if (debug) {
      
    test_function1();
    return;
  }
  
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
  //delay (50);
}




