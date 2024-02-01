# IOTSwitch Project

To Control Switch Device

## IDF_VERSION V5.1

## Mqtt

### Publish

| topic               | message      | format                   |
|---------------------|--------------|--------------------------|
| devices/{SN}/status | DeviceStatus | {"status":bool}          |
| devices/{SN}/config | DeviceConfig | {"config...":"value..."} |

### Subscribe

| topic                | message       | format                     |
|----------------------|---------------|----------------------------|
| devices/{SN}/setting | DeviceSetting | {"setting...": "value..."} |
| devices/{SN}/action  | DeviceAction  | {"action": "..."}          |

#### Action List

| action         | value     | sub data         | announce       |
|----------------|-----------|------------------|----------------|
| set status     | setStatus | {"status": bool} | NON            |
| system restart | restart   | NON              | NON            |
| get status     | getStatus | NON              | publish status |
| get config     | getConfig | NON              | publish config |

## Hardware V0.5

| Pin | Target   | Announce     |
|-----|----------|--------------|
| P4  | NTC_R3   | ADC1_CH3     |
| P5  | CURR_SEN | ADC1_CH4     |
| P6  | NTC_R1   | ADC1_CH5     |
| P7  | TOUCH    | NAN          |
| P8  | WS2812   | NAN          |
| P13 | BUZZER   | NAN          |
| P16 | OUT_PORT | NAN          |
| P17 | OUT_PORT | NAN          |
| P18 | RELAY    | HIGH TO OPEN |
| P2  | MIC_SD   | NAN          |
| P41 | MIC_SCK  | NAN          |
| P42 | MIC_WS   | NAN          |

## Feature

* [x] MQTT control device
* [x] Touchpad control
* [x] Voice control
* [x] WebControl

## Support Target

* [x] ESP32-S3 in process
* [ ] ESP32-C3 wait process
