/*==========================
      INPUT FUNCTIONS     
==========================*/

/*
 * Refresh input values
 * 
 * params: none
 * 
 * return: none
 */
void refreshInputs() {
  if (chamberProbe.isEnabled) {
    float tempT = getTemperatureBySensor(chamberProbe.deviceAddress);
    if (isValidTemperature(tempT)) {
      chamberProbe.value = tempT;
      if (!thermowellProbe.isEnabled) readFailCount = 0;
    } else {
      if (!thermowellProbe.isEnabled) readFailCount++;
    }
  }  
  if (thermowellProbe.isEnabled) {
    float tempT = getTemperatureBySensor(thermowellProbe.deviceAddress);
    if (isValidTemperature(tempT)) {
      thermowellProbe.value = tempT;
      readFailCount = 0;
    } else {
      readFailCount++;
    }
  }
}

/*
 * Read temperature value
 * 
 * params: object
 * device - memory address of the particular sensor to read
 * 
 * return: float
 * - temperature in either Fahrenheit or Celcius depending on units variable
 */
float getTemperatureBySensor(DeviceAddress device) {
  float tempC = sensors.getTempC(device);
  float tempF = DallasTemperature::toFahrenheit(tempC);
  return (units == 'F') ? tempF: tempC;
}

/*
 * Check if temperature value is a number and within a specified plausible range
 * 
 * params: float
 * temperature - sensor input temperature
 * 
 * return: boolean
 * - true if the temperature is a number and within a plausible range
 */
bool isValidTemperature(float temperature) {
  if (units == 'F') {
    if (!isnan(temperature) && temperature < 120 && temperature > 0) return true;
  } else {
    if (!isnan(temperature) && temperature < 50 && temperature > -15) return true;
  }
  return false;
}

