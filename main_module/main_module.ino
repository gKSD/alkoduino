#include <Strela.h>
#include  <Wire.h>

#define LEFT_SENSOR_PIN  8
#define RIGHT_SENSOR_PIN 9

#define SPEED            50//60
//#define SLOW_SPEED       35//7 
//#define BACK_SLOW_SPEED  30//5
//#define BACK_FAST_SPEED  50//10

#define SLOW_SPEED       45//60 
#define BACK_SLOW_SPEED  45//60
#define BACK_FAST_SPEED  50//60


#define BRAKE_K          4

#define STATE_FORWARD    0
#define STATE_RIGHT      1
#define STATE_LEFT       2

#define SPEED_STEP       2

#define FAST_TIME_THRESHOLD     500

#define RIGHT_THRESHOLD  100
#define LEFT_THRESHOLD  200
#define LEFT_SENSOR P12
#define RIGHT_SENSOR P11

int state = STATE_FORWARD;
int currentSpeed = SPEED;
int fastTime = 0;
int prev_right = 0;
int prev_left = 0;

int buttle_counter = 0;

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
    
    uDigitalWrite(L1, LOW);
    uDigitalWrite(L2, HIGH);
    uDigitalWrite(L3, HIGH);
    uDigitalWrite(L4, LOW);
 
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
    
    uDigitalWrite(L1, LOW);
    uDigitalWrite(L2, LOW);
    uDigitalWrite(L3, LOW);
    uDigitalWrite(L4, HIGH);
    
    drive (0, SPEED);
}

void steerLeft() 
{
    state = STATE_LEFT;
    fastTime = 0;

    uDigitalWrite(L1, HIGH);
    uDigitalWrite(L2, LOW);
    uDigitalWrite(L3, LOW);
    uDigitalWrite(L4, LOW);

    //analogWrite(SPEED_LEFT, 0);
    //analogWrite(SPEED_RIGHT, SPEED);

    //digitalWrite(DIR_LEFT, HIGH);
    //digitalWrite(DIR_RIGHT, HIGH);
  
  
    drive(SPEED, 0);
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

    uDigitalWrite(L1, HIGH);
    uDigitalWrite(L2, HIGH);
    uDigitalWrite(L3, HIGH);
    uDigitalWrite(L4, HIGH);    
    drive(rightSpeed, leftSpeed);

    delay(duration);
}


void setup() 
{
    uDigitalWrite(L1, LOW);
    uDigitalWrite(L2, LOW);
    uDigitalWrite(L3, LOW);
    uDigitalWrite(L4, LOW);
    Serial.begin(9600);
    
    int prev_left = uAnalogRead(LEFT_SENSOR);
    int prev_right = uAnalogRead(RIGHT_SENSOR);
    
    analogReference(DEFAULT);
} 

void run_drinks()
{}


void start_moving()
{
    int cur_left, cur_right;
    int ldelta, rdelta;
    boolean left, right;

    prev_left = uAnalogRead(LEFT_SENSOR);
    prev_right = uAnalogRead(RIGHT_SENSOR);

    while(true)
    {
      runForward();
      cur_left = uAnalogRead(LEFT_SENSOR);
      cur_right = uAnalogRead(RIGHT_SENSOR);
      
      ldelta = cur_left - prev_left;
      rdelta = cur_right - prev_right;
      
      right = abs(rdelta) > 30 && rdelta > 0;
      left = abs(ldelta) > 30 && ldelta > 0;
      
      if (right || left)
        return;
    }
}

void test_function() {
     boolean left, right = 0;

    int cur_left, cur_right;
    int ldelta, rdelta;

    while (true)
    {
      delay (1000);
      
      cur_left = uAnalogRead(LEFT_SENSOR);
      cur_right = uAnalogRead(RIGHT_SENSOR);
      
      ldelta = cur_left - prev_left;
      rdelta = cur_right - prev_right;
      
      right = abs(rdelta) > 30 && rdelta < 0;
      left = abs(ldelta) > 30 && ldelta < 0;
      
      Serial.println("*** left -->");
      Serial.println(cur_left);
      Serial.println(prev_left);
      Serial.println(ldelta);
      Serial.println(left);
      Serial.println("*** right -->");
      Serial.println(cur_right);
      Serial.println(prev_right);
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
        uDigitalWrite(L1, HIGH);
        uDigitalWrite(L2, LOW);
        uDigitalWrite(L3, LOW);
        uDigitalWrite(L4, LOW);
      }
      if (right)
      {
        uDigitalWrite(L1, LOW);
        uDigitalWrite(L2, LOW);
        uDigitalWrite(L3, LOW);
        uDigitalWrite(L4, HIGH);
      }
      if (left == right) {
            uDigitalWrite(L1, LOW);
            uDigitalWrite(L2, HIGH);
            uDigitalWrite(L3, HIGH);
            uDigitalWrite(L4, LOW);
      }
      delay (3000);
    }
}

void loop()
{
  boolean debug = false;
  if (debug)
    test_function();
   else
   {
      //boolean left = !digitalRead(LEFT_SENSOR_PIN);
      //boolean right = !digitalRead(RIGHT_SENSOR_PIN);
      boolean left, right = 0;
  
      int cur_left, cur_right;
      int ldelta, rdelta;
   
      cur_left = uAnalogRead(LEFT_SENSOR);
      cur_right = uAnalogRead(RIGHT_SENSOR);
        
      ldelta = cur_left - prev_left;
      rdelta = cur_right - prev_right;
        
      right = abs(rdelta) > 5 && rdelta < 0;
      left = abs(ldelta) > 5 && ldelta < 0;
        
      if (!left) prev_left = cur_left;
      if (!right) prev_right = cur_right;
        
      int targetState;
      
      if (left == right) {
        targetState = STATE_FORWARD;
/*        if (ldelta < 0 && rdelta < 0)
        {
          drive(0,0); //Р С•РЎРѓРЎвЂљР В°Р Р…Р С•Р Р†Р С”Р В°
          run_drinks();
          start_moving(); //Р Р…Р В°РЎвЂЎР В°Р В»Р С• Р Т‘Р Р†Р С‘Р В¶Р ВµР Р…Р С‘РЎРЏ
        }
        else
        {
          targetState = STATE_FORWARD;
        }
*/
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
                    Serial.println(prev_right);
              steerRight();
              break;
  
          case STATE_LEFT:
                    Serial.println("*** LEFT");
                    Serial.println(cur_left);
                    Serial.println(prev_left);
              steerLeft();
              break;
      }
   }
   //delay (10);
}




