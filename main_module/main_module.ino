#include <Alkoduino.h>
#include  <Wire.h>

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

#define TOTAL_STOP_NUMBER

int state = STATE_FORWARD; //STATE_HOME;
int currentSpeed = SPEED;
int fastTime = 0;
int prev_right = 0;
int prev_left = 0;

int first_left_state = 0;
int first_right_state = 0;

int buttle_counter = 0;
int is_stop = 0;
int is_home = 0;

void runForward() 
{
    state = STATE_FORWARD;
    fastTime += 1;
    if (fastTime < FAST_TIME_THRESHOLD) {
        currentSpeed = SLOW_SPEED;
    } else {
        currentSpeed = min(currentSpeed + SPEED_STEP, SPEED);
    }

    //analogWrite(SPEED_LEFT, currentSpeed);
    //analogWrite(SPEED_RIGHT, currentSpeed);

    //digitalWrite(DIR_LEFT, HIGH);
    //digitalWrite(DIR_RIGHT, HIGH);
    
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

    // Р В Р’В Р Р†Р вЂљРІР‚СњР В Р’В Р вЂ™Р’В°Р В Р’В Р РЋРїС—Р…Р В Р’В Р вЂ™Р’ВµР В Р’В Р СћРІР‚пїЅР В Р’В Р вЂ™Р’В»Р В Р Р‹Р В Р РЏР В Р’В Р вЂ™Р’ВµР В Р’В Р РЋРїС—Р… Р В Р’В Р РЋРІР‚вЂќР В Р Р‹Р В РІР‚С™Р В Р’В Р вЂ™Р’В°Р В Р’В Р В РІР‚В Р В Р’В Р РЋРІР‚СћР В Р’В Р вЂ™Р’Вµ Р В Р’В Р РЋРІР‚СњР В Р’В Р РЋРІР‚СћР В Р’В Р вЂ™Р’В»Р В Р’В Р вЂ™Р’ВµР В Р Р‹Р В РЎвЂњР В Р’В Р РЋРІР‚Сћ Р В Р’В Р РЋРІР‚СћР В Р Р‹Р Р†Р вЂљРЎв„ўР В Р’В Р В РІР‚В¦Р В Р’В Р РЋРІР‚СћР В Р Р‹Р В РЎвЂњР В Р’В Р РЋРІР‚пїЅР В Р Р‹Р Р†Р вЂљРЎв„ўР В Р’В Р вЂ™Р’ВµР В Р’В Р вЂ™Р’В»Р В Р Р‹Р В Р вЂ°Р В Р’В Р В РІР‚В¦Р В Р’В Р РЋРІР‚Сћ Р В Р’В Р вЂ™Р’В»Р В Р’В Р вЂ™Р’ВµР В Р’В Р В РІР‚В Р В Р’В Р РЋРІР‚СћР В Р’В Р РЋРІР‚вЂњР В Р’В Р РЋРІР‚Сћ,
    // Р В Р Р‹Р Р†Р вЂљР Р‹Р В Р Р‹Р Р†Р вЂљРЎв„ўР В Р’В Р РЋРІР‚СћР В Р’В Р вЂ™Р’В±Р В Р Р‹Р Р†Р вЂљРІвЂћвЂ“ Р В Р’В Р В РІР‚В¦Р В Р’В Р вЂ™Р’В°Р В Р Р‹Р Р†Р вЂљР Р‹Р В Р’В Р вЂ™Р’В°Р В Р Р‹Р Р†Р вЂљРЎв„ўР В Р Р‹Р В Р вЂ° Р В Р’В Р РЋРІР‚вЂќР В Р’В Р РЋРІР‚СћР В Р’В Р В РІР‚В Р В Р’В Р РЋРІР‚СћР В Р Р‹Р В РІР‚С™Р В Р’В Р РЋРІР‚СћР В Р Р‹Р Р†Р вЂљРЎв„ў
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

    Serial.print ("STEP BACK");
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


void setup() 
{
    u_digital_write(L1, LOW);
    u_digital_write(L2, LOW);
    u_digital_write(L3, LOW);
    u_digital_write(L4, LOW);
    Serial.begin(9600);
    
    int prev_left = u_analog_read(LEFT_SENSOR);
    int prev_right = u_analog_read(RIGHT_SENSOR);
    
    analogReference(DEFAULT);
} 

void run_drinks()
{}


void start_moving()
{
    int cur_left, cur_right;
    int ldelta, rdelta;
    boolean left, right;

    prev_left = u_analog_read(LEFT_SENSOR);
    prev_right = u_analog_read(RIGHT_SENSOR);

    while(true)
    {
      runForward();
      cur_left = u_analog_read(LEFT_SENSOR);
      cur_right = u_analog_read(RIGHT_SENSOR);
      
      ldelta = cur_left - prev_left;
      rdelta = cur_right - prev_right;
      
      right = abs(rdelta) > 30 && rdelta > 0;
      left = abs(ldelta) > 30 && ldelta > 0;
      
      if (right || left)
        return;
    }
}

/*void run_home()
{
  while (is_home) {
    
  }
}*/

void check_state () {
      boolean left, right = 0;
      int cur_left, cur_right;
      int ldelta, rdelta;
      
      cur_left = u_analog_read(LEFT_SENSOR);
      cur_right = u_analog_read(RIGHT_SENSOR);
      
      
      if( first_left_state == 0) first_left_state = cur_left;
      if ( first_right_state == 0) first_right_state = cur_right;
      
      ldelta = cur_left - first_left_state;
      rdelta = cur_right - first_right_state;

      right = rdelta < -75;
      left = ldelta < -75;
      
     /* if (left == 0 && right == 0) {
        first_left_state = cur_left;
        first_right_state = cur_right;
      }*/ 
      int targetState;
      
      if (left == right) {
        if (left && right)
        {
          Serial.println("*** STOPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP *************************** ");
          drive(0,0);
          is_stop = 1;
          targetState = STATE_STOP;
          //run_drinks();
          //start_moving();
        }
        else
        {
          targetState = STATE_FORWARD;
        }
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
              Serial.println("*** FORWARD");
              Serial.println(cur_left);
              Serial.println(cur_right);
              runForward();
              break;
  
          case STATE_RIGHT:
                    Serial.println("*** RIGHT");
                    Serial.println(cur_right);
                    Serial.println(first_right_state);
              steerRight();
              break;
  
          case STATE_LEFT:
                    Serial.println("*** LEFT");
                    Serial.println(cur_left);
                    Serial.println(first_left_state);
              steerLeft();
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
      
      Serial.println("*** left -->");
      Serial.println(cur_left);
      Serial.println(first_left_state);
      Serial.println(ldelta);
      Serial.println(left);
      Serial.println("*** right -->");
      Serial.println(cur_right);
      Serial.println(first_right_state);
      Serial.println(rdelta);
      Serial.println(right);
      
      if (!left) prev_left = cur_left;
      if (!right) prev_right = cur_right;
      
      //Serial.println("left -->");
      //Serial.println(left);
      //Serial.println("right -->");
      //Serial.println(right);
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

void loop()
{
  boolean debug = false; 
  if (debug) {
    test_function();
    return;
  }
/*    
   if (state == STATE_HOME) {
     is_home = 1;
     run_home();
   }   
   if (is_stop == 0)
   {
*/
     check_state();
/*
   }
   
   if (is_stop) {
     //buttle counter
     //TOTAL_STOP_NUMBER
   }
   
  delay (50);
  */
}




