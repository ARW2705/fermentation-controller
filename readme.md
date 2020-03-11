# Fermentation Temperature Controller

---

This application and hardware setup provides a means to
control cooling, heating, and ventilation with an
insulated fermentation control chamber such as a small fridge.

*Work in progress. Currently undergoing live tests. Project updates will follow*


## Getting Started

---

### Prerequisites

To clone and run this application, you'll need [Git](https://git-scm.com/), [Arduino](https://www.arduino.cc/), and either [Arduino IDE](https://www.arduino.cc/en/Main/Software) or another compatible arduino compiler and loader.

### Hardware

* NodeMCU v1.0
* 20x4 I2C LCD
* 4 Channel opto-isolated relay block
* DS18B20 waterproof temperature sensors x2
* Push buttons x4
* 5V wall adapter
* Female barrel power jack
* LD1117V33 voltage regulator
* Various capacitors, resistors, and wiring

![Fritzing breadboard diagram](fermentation_chamber_bb_v1.png)

### Usage

By default, the chamber temperature sensor, cooling system,
and heating system are enabled.  The cooling and heating
systems may be enabled/disabled independently.  If enabled,
the thermowell temperature sensor becomes the system's
operating input, the chamber sensor will still report it's
readings.

## Future Plans

---

### Features

* Integrate with [BrewIO](https://github.com/ARW2705/BrewIO-App)
* Enable flightrecorder for analytics
* Operate with premade control schedules

### Improvements

* Refactor with singleton pattern
* Update docs


## Built With

---

* [Arduino](https://www.arduino.cc/) - Micro controller


## License

---

This project is licensed under the MIT License - see the [LICENSE](https://github.com/ARW2705/Fermentation-Control/blob/master/LICENSE) file for details.
