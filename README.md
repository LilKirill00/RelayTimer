# RelayTimer

## Описание
Программа на Arduino, которая позволяет настроить время запуска и выключения реле по таймеру.

При запуске, если есть время до отключения, то реле запустится и дальше будет работать по таймеру

## Первый запуск
Настройте в коде через какой пин идет сигнал к реле (по умолчанию 2)

Загрузите код на Arduino

В монитор порта (порт 9600) выполнить следующие команды:
1. `setTimeRTC <hours> <minutes> <seconds>` - указав текущее время (не нужно если уже настроено время на RTC)
2. `setStart <hours> <minutes>` - установить время запуска реле (по умолчанию будет 23:59)
3. `setEnd <hours> <minutes>` - установить время выключения реле (по умолчанию будет 23:59)

Все настройки должны быть установлены: для проверки можете воспользоваться:
- `getConf`
- `getTimeRTC`

## Доступные команды

|Команда|Описание|
|---|---|
|help|вывести список доступных команд|
|getConf| вывести время запуска и выключения|
|getStart| вывести время запуска|
|getEnd| вывести время выключения|
|getTimeRTC| вывести текущее время на RTC|
|||
|setTimeRTC <hours> <minutes> <seconds>| установить время на RTC|
|setStart <hours> <minutes>| установить время запуска|
|setEnd <hours> <minutes>| установить время выключения|
|||
|getState| вывести состояние реле|
|relayOn| включить реле|
|relayOff| выключить реле|

## Использованные компоненты
- Arduino Nano - R3
- RTC DS3231
- Relay Module

## Используемые библиотеки
- RTClib
- GyverTimer
