// Настройки которые можно менять

#define RELAY_PIN 2    // пин для реле на который подавать ток чтобы вкл или выключить
#define RELAY_TYPE LOW  // какая должна быть сила сигнала для вкл реле (LOW или HIGH)

// Конец настроек


void showHelp() {
  Serial.println(F("help                                        - вывести список доступных команд"));
  Serial.println(F("getConf                                     - вывести время запуска и выключения"));
  Serial.println(F("getStart                                    - вывести время запуска"));
  Serial.println(F("getEnd                                      - вывести время выключения"));
  Serial.println(F("getTimeRTC                                  - вывести текущее время на RTC"));
  Serial.println(F(""));
  Serial.println(F("setTimeRTC <hours> <minutes> <seconds>      - установить время на RTC"));
  Serial.println(F("setStart <hours> <minutes>                  - установить время запуска"));
  Serial.println(F("setEnd <hours> <minutes>                    - установить время выключения"));
  Serial.println(F(""));
  Serial.println(F("getState                                    - вывести состояние реле"));
  Serial.println(F("relayOn                                     - включить реле"));
  Serial.println(F("relayOff                                    - выключить реле"));
}

// Функция для конвертации часов и минут в минуты
int timeToMinutes(uint8_t hours, uint8_t minutes) {
  return hours * 60 + minutes;
}

// глобальные переменные
uint8_t START_HRS, START_MINS, END_HRS, END_MINS;           // время запуска и выключения
uint8_t hrs = 23, mins = 59, secs = 59;
boolean state;                                              // В каком состояние сейчас реле
int startTime, endTime;                                     // Для хранения посчитаных времени запуска и выключения в минутах

// внутрений таймер по которому можно определить что минута прошла
#include "GyverTimer.h"
GTimer_ms minsTimer((uint16_t)60 * 1000);

// для взаимодействия с платой RTC
#include "RTClib.h"
RTC_DS3231 rtc;

// для работы с постоянной памятью
#include "EEPROM.h"

// Первый запуск
void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);

  // читаем настройки сохраненые
  START_HRS = EEPROM.read(0);
  START_MINS = EEPROM.read(1);
  END_HRS = EEPROM.read(2);
  END_MINS = EEPROM.read(3);
  loadConf();

  // узнаем какое сейчас время
  rtc.begin();
  globTimeFromRTC();

  // запускаем если есть время до выключения
  if (isTimeInInterval()) {
    relayOn();
  } else { // иначе отключаем
    relayOff();
  }
}

// бесконечный цикл работы контроллера
void loop() {
  clockTick();

  // чтение команд
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n'); // Чтение строки до символа новой строки
    command.trim(); // Удаление лишних пробелов

    // ищем делитель аргументов
    int nilComma = command.indexOf(' ');
    int firstComma = command.indexOf(' ', nilComma + 1);
    int secondComma = command.indexOf(' ', firstComma + 1);

    // определяем аргументы
    String cmd, arg1, arg2, arg3;
    if (nilComma != -1) {
      cmd = command.substring(0, nilComma);
      if (firstComma != -1) {
        arg1 = command.substring(nilComma + 1, firstComma);
        if (secondComma != -1) {
          arg2 = command.substring(firstComma + 1, secondComma);
          arg3 = command.substring(secondComma + 1);
        } else {
          arg2 = command.substring(firstComma + 1);
        }
      }
    }

    // Определяем, какую команду ввел пользователь
    if (command.equalsIgnoreCase("getConf")) { // вывести время запуска и выключения
      Serial.println("START_HRS=" + String(START_HRS, 10) + " ; START_MINS=" + String(START_MINS, 10));
      Serial.println("END_HRS=" + String(END_HRS, 10) + " ; END_MINS=" + String(END_MINS, 10));

    } else if (command.equalsIgnoreCase("getStart")) { // вывести время запуска
      Serial.println("START_HRS=" + String(START_HRS, 10) + " ; START_MINS=" + String(START_MINS, 10));

    } else if (command.equalsIgnoreCase("getEnd")) { // вывести время выключения
      Serial.println("END_HRS=" + String(END_HRS, 10) + " ; END_MINS=" + String(END_MINS, 10));

    } else if (command.equalsIgnoreCase("getTimeRTC")) { // вывести текущее время на RTC
      Serial.println("Время на RTC:" + String(hrs, 10) + ":" + String(mins, 10) + ":" + String(secs, 10));

    } else if (command.startsWith("setTimeRTC")) { // установить время на RTC
      if (arg1 != "" && arg2 != "" && arg3 != "") {
        rtc.adjust(DateTime(2014, 1, 21, arg1.toInt(), arg2.toInt(), arg3.toInt())); // установка нового времени в RTC
        Serial.println(F("Время установлено"));
      } else {
        Serial.println(F("Недостаточно аргументов"));
      }

    } else if (command.startsWith("setStart")) { // установить время запуска
      setTimeConf(START_HRS, START_MINS, 0, 1, arg1, arg2);

    } else if (command.startsWith("setEnd")) { // установить время выключения
      setTimeConf(END_HRS, END_MINS, 2, 3, arg1, arg2);

    } else if (command.equalsIgnoreCase("getState")) { // вывести состояние реле
      Serial.println(state);

    } else if (command.equalsIgnoreCase("relayOn")) { // включить реле
      relayOn();

    } else if (command.equalsIgnoreCase("relayOff")) { // выключить реле
      relayOff();

    } else if (command.equalsIgnoreCase("help")) {
      showHelp();

    } else {
      Serial.println(command + ": команда не найдена");
    }
  }
}

// установка времени в конфиги
void setTimeConf(uint8_t& hrsVariable, uint8_t& minsVariable, int eepromIndexHrs, int eepromIndexMins, String arg1, String arg2) {
  if (arg1 != "" && arg2 != "") {
    hrsVariable = arg1.toInt();
    minsVariable = arg2.toInt();
    EEPROM.update(eepromIndexHrs, hrsVariable);
    EEPROM.update(eepromIndexMins, minsVariable);
    loadConf();
    Serial.println(F("Время установлено"));
  } else {
    Serial.println(F("Недостаточно аргументов"));
  }
}

// настроить время запуска и выключения
void loadConf() {
  START_HRS = constrain(START_HRS, 0, 23);
  START_MINS = constrain(START_MINS, 0, 59);
  END_HRS = constrain(END_HRS, 0, 23);
  END_MINS = constrain(END_MINS, 0, 59);
  startTime = timeToMinutes(START_HRS, START_MINS);
  endTime = timeToMinutes(END_HRS, END_MINS);
  if (startTime == endTime) {
    Serial.println(F("Время запуска и отключения одинаковые"));
  }
}

// установить глобально настройки взяв значения из RTC
void globTimeFromRTC() {
  DateTime now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();
}

// определить в каком интервале время
bool isTimeInInterval() {
  int currentTime = timeToMinutes(hrs, mins);

  // Если конечное время меньше начального, значит интервал переходит через полночь
  if (endTime < startTime) {
    return (currentTime >= startTime || currentTime < endTime);
  } else {
    return (currentTime >= startTime && currentTime < endTime);
  }
}

// счетчик времени
void clockTick() {
  // узнаем время
  globTimeFromRTC();

  // запускаем каждую минуту
  if (minsTimer.isReady()) {
    // если настало время выключения
    if (state && ((mins == END_MINS && hrs == END_HRS) || !isTimeInInterval())) {
      relayOff();
      return;
    }

    // если настало время включения
    if (!state && ((mins == START_MINS && hrs == START_HRS) || isTimeInInterval())) {
      relayOn();
      return;
    }
  }
}

// включить реле
void relayOn() {
  Serial.println(F("Включение реле..."));
  state = true;
  if (RELAY_TYPE == LOW) {
    digitalWrite(RELAY_PIN, LOW);
  } else {
    digitalWrite(RELAY_PIN, HIGH);
  }
}

// Выключить реле
void relayOff() {
  Serial.println(F("Выключение реле..."));
  state = false;
  if (RELAY_TYPE == LOW) {
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
}
