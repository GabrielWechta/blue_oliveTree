#include <Arduino.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK_W 2
#define DIO_W 3

#define CLK_B 6
#define DIO_B 7

TM1637Display display_W(CLK_W, DIO_W);
TM1637Display display_B(CLK_B, DIO_B);

const int whiteButtonPin = 4;
const int blackButtonPin = 5;
const int okButtonPin = 8;
const int upButtonPin = 9;
int buttonState = 0;
int W[4];
int B[4];
uint8_t data_W[] = { 0xff, 0xff, 0xff, 0xff };
uint8_t data_B[] = { 0xff, 0xff, 0xff, 0xff };
uint8_t DEAD[] = { 94, 121, 119, 94 };
bool button_pressed = false;

class Timer {
  private:
    bool time_up = false;
    int timeTable[4];
    char kind = 'N'; //var to recognize type of Timer inside second() method.

  public:
    int time;
    Timer(int time, char kind);
    bool second();
    void copyTimeTable(int* table);
    void addMinute();
};

Timer::Timer(int time, char kind) {
  this->time = time;
  this->kind = kind;
  for (int i = 3; i >= 0; i--) {
    timeTable[i] = time % 10;
    time /= 10;
  }
}

void Timer::copyTimeTable(int* outside) {
  for (int i = 0; i < 4; i++) {
    outside[i] = timeTable[i];
  }
}

bool Timer::second() {
  int whiteButtonState = LOW;
  int blackButtonState = LOW;

  for (int i = 0; i < 100; i++) {
    delay(10);
    whiteButtonState = digitalRead(whiteButtonPin);
    blackButtonState = digitalRead(blackButtonPin);

    if ((whiteButtonState == HIGH && this->kind == 'W') || (blackButtonState == HIGH && this->kind == 'B')) {
      whiteButtonState = LOW;
      blackButtonState = LOW;
      return true;
    }
  }

  if (time % 100 == 0) time -= 40;

  time--;
  int tmp = time;

  for (int i = 3; i >= 0; i--) {
    timeTable[i] = tmp % 10;
    tmp /= 10;
  }

  if (time % 1000 == 0) time_up = true;

  return false;
}

void Timer::addMinute() {
  this->time += 100;
  for (int i = 3; i >= 0; i--) {
    timeTable[i] = time % 10;
    time /= 10;
  }
}

Timer WhiteTimer = Timer(0, 'W');
Timer BlackTimer = Timer(0, 'B');


void showWhite() {
  WhiteTimer.copyTimeTable(W);
  data_W[0] = display_W.encodeDigit(W[0]);
  data_W[1] = display_W.encodeDigit(W[1]);
  data_W[2] = display_W.encodeDigit(W[2]);
  data_W[3] = display_W.encodeDigit(W[3]);
  display_W.setSegments(data_W);
}

void showBlack() {
  BlackTimer.copyTimeTable(B);
  data_B[0] = display_B.encodeDigit(B[0]);
  data_B[1] = display_B.encodeDigit(B[1]);
  data_B[2] = display_B.encodeDigit(B[2]);
  data_B[3] = display_B.encodeDigit(B[3]);
  display_B.setSegments(data_B);

}

void stop() {
  while (true);
}

void timeSetup(Timer WhiteTimer, Timer BlackTimer) {
  int upButtonState = LOW;
  int okButtonState = LOW;

  while (okButtonState == LOW) {
    showWhite();
    if (upButtonState == HIGH) {
      WhiteTimer.addMinute();
    }
    upButtonState = digitalRead(upButtonPin);
    okButtonState = digitalRead(okButtonPin);
  }

  upButtonState = LOW;
  okButtonState = LOW;
  
  while (okButtonState == LOW) {
    showBlack();
    if (upButtonState == HIGH) {
      BlackTimer.addMinute();
    }
    upButtonState = digitalRead(upButtonPin);
    okButtonState = digitalRead(okButtonPin);
  }
}

void setup()
{
  pinMode(whiteButtonPin, INPUT);
  pinMode(blackButtonPin, INPUT);
  pinMode(okButtonPin, INPUT);
  pinMode(upButtonPin, INPUT);

  display_W.setBrightness(0x0f);
  display_B.setBrightness(0x0f);

  Serial.begin(9600);

  timeSetup(WhiteTimer, BlackTimer);
}

void loop()
{

  while (1) {
    button_pressed = WhiteTimer.second();
    showWhite();

    if (WhiteTimer.time % 10000 == 0) {
      display_W.setSegments(DEAD);
      stop();
    }

    if (button_pressed == true) {
      button_pressed = false;
      break;
    }
  }

  delay(300);

  while (1) {
    button_pressed = BlackTimer.second();
    showBlack();

    if (BlackTimer.time % 10000 == 0) {
      display_B.setSegments(DEAD);
      stop();
    }

    if (button_pressed == true) {
      button_pressed = false;
      break;
    }
  }

  delay(300);
}
