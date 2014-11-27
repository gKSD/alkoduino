#define LEFT_SENSOR_PIN   8
#define RIGHT_SENSOR_PIN  9
#define SPEED_LEFT        6
#define SPEED_RIGHT       5 
#define DIR_LEFT          7
#define DIR_RIGHT         4

// Для того чтобы убедиться, что именно тормозной путь долог, а не команда остановиться 
// приходит слишком поздно, будем включать светодиод, когда отдаётся команда.
#define LED_PIN           13

int currSpeed = 40;
void setup()
{
    for(int i = 4; i <= 7; ++i)
        pinMode(i, OUTPUT);

    analogWrite(SPEED_RIGHT, currSpeed);
    digitalWrite(DIR_RIGHT, HIGH);

    analogWrite(SPEED_LEFT, currSpeed);
    digitalWrite(DIR_LEFT, HIGH);    

    pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    if (currSpeed > 120)
        return;

    boolean white[] = {
        !digitalRead(LEFT_SENSOR_PIN), 
        !digitalRead(RIGHT_SENSOR_PIN)
    };

    if (white[0] && white[1]) {
        // едем пока не упрёмся
        return;
    }

    // зажигаем светодиод, останавливаем моторы
    // и наблюдаем
    digitalWrite(LED_PIN, HIGH);
    analogWrite(SPEED_RIGHT, 0);
    analogWrite(SPEED_LEFT, 0);
    delay(5000);

    // повторяем эксперимент, увеличивая скорость
    // на 10 пунктов
    currSpeed += 10;
    if (currSpeed > 120)
        return;

    digitalWrite(LED_PIN, LOW);
    analogWrite(SPEED_RIGHT, currSpeed);
    analogWrite(SPEED_LEFT, currSpeed);    
}