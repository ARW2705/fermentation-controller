/*==========================
      OUTPUT FUNCTIONS     
==========================*/

/*
 * Refresh the output handlers
 * 
 * params: none
 * 
 * return: none
 */
void refreshOutputs() {
  if (chamberProbe.isEnabled || thermowellProbe.isEnabled) {
    runThermostatCycle();
  } else {
    deactivateAllCycles();
    timer.coolDownStart = millis();
    timer.circulationStart = millis();
  }
}

/*
 * Update thermostat cycle
 * Called at specified intervals, if no changes are to be made,
 * the same values will be input again
 * 
 * params: none
 * 
 * return: none
 */
void runThermostatCycle() {
  if ((millis() - timer.coolDownStart > COOLDOWN_INTERVAL) // check if cooldown has completed
      && isValidTemperature(monitored->value) // check if monitored temperature value is plausible
      && (readFailCount < READ_FAIL_MAX)) { // check if temperature data is current
    // start cooling cycle if temperature is too high
    if (cooling.isEnabled && (monitored->value - monitored->target) > monitored->tolerance) {
      cooling.target = 1;
      heating.target = 0;
      fan.target = 1;
      runCoolingCycle();
    // start heating cycle if temperature is too low
    } else if (heating.isEnabled && (monitored->target - monitored->value) > monitored->tolerance) {
      cooling.target = 0;
      heating.target = 1;
      fan.target = 1;
      runHeatingCycle();
    // if cooling and current temperature is less than target, 
    // or if heating and current temperature is greater than target, 
    // or if previously cooling and cooling has become disabled,
    // or if previously heating and heating has become disabled,
    // end current cycle 
    } else if ((cooling.target && monitored->value < monitored->target)
               || (heating.target && monitored->value > monitored->target)
               || (cooling.target && !cooling.isEnabled)
               || (heating.target && !heating.isEnabled)) {
      timer.coolDownStart = millis();
      deactivateAllCycles();
    } 
  } else {
    // if the cooling or heating cycles were previously on, 
    // deactivate them and restart the cooldown
    if (cooling.target || heating.target) {
      timer.coolDownStart = millis();
      deactivateAllCycles();
    }
  }
}

/*
 * Update output values when a cycle is changed
 * 
 * params: none
 * 
 * return: none
 */
void updateOutputValues() {
  cooling.value = cooling.target;
  heating.value = heating.target;
  fan.value = fan.target;
}

/*
 * Activate the heating element and fan outputs
 * 
 * params: none
 * 
 * return: none
 */
void runHeatingCycle() {
  if (!heating.value) {
    digitalWrite(COOL_RELAY, HIGH);
    digitalWrite(HEAT_RELAY, LOW);
    digitalWrite(FAN_RELAY, LOW);
    updateOutputValues();
  }
  timer.circulationStart = millis();
}

/*
 * Activate the cooling compressor and fan outputs
 * 
 * params: none
 * 
 * return: none
 */
void runCoolingCycle() {
  if (!cooling.value) {
    digitalWrite(COOL_RELAY, LOW);
    digitalWrite(HEAT_RELAY, HIGH);
    digitalWrite(FAN_RELAY, LOW);
    updateOutputValues();
  }
  timer.circulationStart = millis();
}

/*
 * Activate fan outputs only
 * 
 * params: none
 * 
 * return: none
 */
void runCirculationCycle() {
  updateOutputValues();
  digitalWrite(FAN_RELAY, LOW);
}

/*
 * Deactivate fan outputs only
 * 
 * params: none
 * 
 * return: none
 */
void endCirculationCycle() {
  updateOutputValues();
  digitalWrite(FAN_RELAY, HIGH);
}

/*
 * Deactivate all cycles
 * Causes for deactivating cycles:
 *  Normal case
 *    - temperature target has been reached
 *  
 *  Error case
 *    - there are no temperature sensors enabled
 *    - temperature sensor is not plausible
 *    - temperature sensor data is too old
 *    
 * params: none
 * 
 * return: none
 */
void deactivateAllCycles() {
  digitalWrite(COOL_RELAY, HIGH);
  digitalWrite(HEAT_RELAY, HIGH);
  digitalWrite(FAN_RELAY, HIGH);
  heating.target = 0;
  cooling.target = 0;
  fan.target = 0;
  updateOutputValues();
}

