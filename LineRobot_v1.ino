// Моторы подключаются к клеммам M1+,M1-,M2+,M2-  
// Motor shield использует четыре контакта 6,5,7,4 для управления моторами 
#define SPEED_LEFT       6
#define SPEED_RIGHT      5 
#define DIR_LEFT         7
#define DIR_RIGHT        4
#define LEFT_SENSOR_PIN  8
#define RIGHT_SENSOR_PIN 9

// Скорость, с которой мы движемся вперёд (0-255)
#define SPEED            35

// Коэффициент, задающий во сколько раз нужно затормозить
// одно из колёс для поворота
#define BRAKE_K          4

#define STATE_FORWARD    0
#define STATE_RIGHT      1
#define STATE_LEFT       2

int state = STATE_FORWARD;

void runForward() 
{
    state = STATE_FORWARD;

    // Для регулировки скорости `SPEED` может принимать значения от 0 до 255,
    // чем болше, тем быстрее. 
    analogWrite(SPEED_LEFT, SPEED);
    analogWrite(SPEED_RIGHT, SPEED);

    // Если в DIR_LEFT или DIR_RIGHT пишем HIGH, мотор будет двигать соответствующее колесо
    // вперёд, если LOW - назад.
    digitalWrite(DIR_LEFT, HIGH);
    digitalWrite(DIR_RIGHT, HIGH);
}

void steerRight() 
{
    state = STATE_RIGHT;

    // Замедляем правое колесо относительно левого,
    // чтобы начать поворот
    analogWrite(SPEED_RIGHT, SPEED / BRAKE_K);
    analogWrite(SPEED_LEFT, SPEED);

    digitalWrite(DIR_LEFT, HIGH);
    digitalWrite(DIR_RIGHT, HIGH);
}

void steerLeft() 
{
    state = STATE_LEFT;

    analogWrite(SPEED_LEFT, SPEED / BRAKE_K);
    analogWrite(SPEED_RIGHT, SPEED);

    digitalWrite(DIR_LEFT, HIGH);
    digitalWrite(DIR_RIGHT, HIGH);
}


void setup() 
{
    // Настраивает выводы платы 4,5,6,7 на вывод сигналов 
    for(int i = 4; i <= 7; i++)
        pinMode(i, OUTPUT);

    // Сразу едем вперёд
    runForward();
} 

void loop() 
{ 
    // Наш робот ездит по белому полю с чёрным треком. В обратном случае не нужно
    // инвертировать значения с датчиков
    boolean left = !digitalRead(LEFT_SENSOR_PIN);
    boolean right = !digitalRead(RIGHT_SENSOR_PIN);

    // В какое состояние нужно перейти?
    int targetState;

    if (left == right) {
        // под сенсорами всё белое или всё чёрное
        // едем вперёд
        targetState = STATE_FORWARD;
    } else if (left) {
        // левый сенсор упёрся в трек
        // поворачиваем налево
        targetState = STATE_LEFT;
    } else {
        targetState = STATE_RIGHT;
    }

    if (state == targetState) {
        // мы уже делаём всё что нужно,
        // делаем измерения заново
        return;
    }

    switch (targetState) {
        case STATE_FORWARD:
            runForward();
            break;

        case STATE_RIGHT:
            steerRight();
            break;

        case STATE_LEFT:
            steerLeft();
            break;
    }

    // не позволяем сильно вилять на прямой
    delay(50);
}