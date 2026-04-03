
// Quick guide make sure to downlode the zip file of the library EEPROM u can find online.
// don't  forget to follow this account for more iot/robotics stuff --BHUVAN:)
#include <EEPROM.h>

#define IN1 4
#define IN2 5
#define ENA 9
#define IN3 2
#define IN4 3
#define ENB 10

#define ir1 A0
#define ir2 A1
#define ir3 A2
#define ir4 A3
#define ir5 A4

#define MODE_SELECT_PIN 12

#define MAX_PATH_LENGTH 100
#define EEPROM_START_ADDR 0

float Kp = 35.0;
float Kd = 10.0;

const int baseSpeed = 120;
const int maxSpeed = 200;
const int turnSpeed = 150;
const int speedRunBase = 200;

const unsigned long sharpTurnTime = 380;
const unsigned long junctionPivotTime = 400;
const unsigned long postJunctionMove = 150;
const unsigned long uTurnTime = 650;
const unsigned long preJunctionMove = 220;

unsigned long lastTime = 0;
float prevError = 0.0;

int runMode = 1;

char path[MAX_PATH_LENGTH];
char optimized_path[MAX_PATH_LENGTH];

int path_index = 0;
int path_length = 0;
int optimized_path_index = 0;
int optimized_path_length = 0;

void clearEEPROM() {
  for (int i = EEPROM_START_ADDR; i < 512; i++) {
    if (EEPROM.read(i) != 0) EEPROM.write(i, 0);
    else break;
  }
}

void loadPathFromEEPROM() {
  optimized_path_length = 0;
  for (int i = 0; i < MAX_PATH_LENGTH; i++) {
    char turn = EEPROM.read(EEPROM_START_ADDR + i);
    if (turn == 0 || turn == 'E') break;
    optimized_path[i] = turn;
    optimized_path_length++;
  }
}

void optimizeAndSavePath() {
  for (int i = 0; i < path_length; i++) optimized_path[i] = path[i];
  optimized_path_length = path_length;

  bool changes = true;

  while (changes) {
    changes = false;
    char temp[MAX_PATH_LENGTH];
    int idx = 0;

    for (int i = 0; i < optimized_path_length; i++) {
      if (i + 2 < optimized_path_length && optimized_path[i + 1] == 'U') {
        char t1 = optimized_path[i];
        char t2 = optimized_path[i + 2];

        if (t1 == 'L' && t2 == 'L') { temp[idx] = 'U'; i += 2; changes = true; }
        else if (t1 == 'L' && t2 == 'S') { temp[idx] = 'R'; i += 2; changes = true; }
        else if (t1 == 'S' && t2 == 'L') { temp[idx] = 'R'; i += 2; changes = true; }
        else if (t1 == 'S' && t2 == 'S') { temp[idx] = 'U'; i += 2; changes = true; }
        else if (t1 == 'L' && t2 == 'R') { temp[idx] = 'S'; i += 2; changes = true; }
        else if (t1 == 'R' && t2 == 'L') { temp[idx] = 'S'; i += 2; changes = true; }
        else if (t1 == 'S' && t2 == 'R') { temp[idx] = 'L'; i += 2; changes = true; }
        else if (t1 == 'R' && t2 == 'S') { temp[idx] = 'L'; i += 2; changes = true; }
        else temp[idx] = optimized_path[i];
      } else {
        temp[idx] = optimized_path[i];
      }
      idx++;
    }

    for (int j = 0; j < idx; j++) optimized_path[j] = temp[j];
    optimized_path_length = idx;
  }

  if (optimized_path_length > 0 && optimized_path[optimized_path_length - 1] == 'E')
    optimized_path_length--;

  for (int i = 0; i < optimized_path_length; i++)
    EEPROM.write(EEPROM_START_ADDR + i, optimized_path[i]);

  EEPROM.write(EEPROM_START_ADDR + optimized_path_length, 'E');
}

void setMotors(int l, int r) {
  l = constrain(l, 0, 255);
  r = constrain(r, 0, 255);

  analogWrite(ENB, l);
  analogWrite(ENA, r);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void pivotRight(int pwm) {
  analogWrite(ENA, pwm);
  analogWrite(ENB, pwm);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void pivotLeft(int pwm) {
  analogWrite(ENA, pwm);
  analogWrite(ENB, pwm);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void performUTurn() {
  pivotRight(turnSpeed);
  delay(uTurnTime);
  prevError = 0;
}

void setup() {
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  pinMode(ir1, INPUT); pinMode(ir2, INPUT);
  pinMode(ir3, INPUT); pinMode(ir4, INPUT); pinMode(ir5, INPUT);

  pinMode(MODE_SELECT_PIN, INPUT_PULLUP);

  Serial.begin(115200);

  if (digitalRead(MODE_SELECT_PIN) == LOW) {
    runMode = 1;
    clearEEPROM();
  } else {
    runMode = 2;
    loadPathFromEEPROM();
  }

  lastTime = millis();
}

void loop() {
  int s1 = digitalRead(ir1);
  int s2 = digitalRead(ir2);
  int s3 = digitalRead(ir4);
  int s4 = digitalRead(ir3);
  int s5 = digitalRead(ir5);

  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  if (dt <= 0) dt = 0.001;
  lastTime = now;

  int weights[5] = {-2, -1, 0, 1, 2};
  int sensors[5] = {s1, s2, s3, s4, s5};

  int num = 0, den = 0;
  for (int i = 0; i < 5; i++) {
    num += sensors[i] * weights[i];
    den += sensors[i];
  }

  float error = (den != 0) ? float(num) / den : 0;
  float derivative = (error - prevError) / dt;
  prevError = error;

  float control = Kp * error + Kd * derivative;

  int base = (runMode == 1) ? baseSpeed : speedRunBase;

  int left = constrain(base + control, 0, maxSpeed);
  int right = constrain(base - control, 0, maxSpeed);

  setMotors(left, right);
}
