/*===========================
      LOCAL INTERFACE     
===========================*/

/*
 * Listen for analog input event
 * 
 * params: none
 * 
 * return: none
 */
void listenForButton() {
  int input = analogRead(ANALOG_INPUT);
  if (input > 50) {
    // a button is pressed
    if (analogIndex < SAMPLE_SIZE) {
      // add the value to sampling array
      analogInputs[analogIndex] = input;
      analogIndex++;
    } else {
      // button is being held - handle the input and reset sampling
      handleAnalogInput(normalizeAnalogInputs());
      resetAnalogInput();
    }
  } else {
    if (analogIndex) {
      // button was released - handle the input and reset sampling
      handleAnalogInput(normalizeAnalogInputs());
      resetAnalogInput();
    }
  }
} // end listenForButton

/*
 * Reset the analog sampling index and array
 * 
 * params: none
 * 
 * return: none
 */
void resetAnalogInput() {
  analogIndex = 0;
  for (int i=0; i < SAMPLE_SIZE; i++) {
    analogInputs[i] = 0;
  }
} // end resetAnalogInput


/*
 * Normalize the analog inputs
 * An analog input is considered ok if it is +/- 100 of the specified input midpoints
 * Any input not meeting this specification is not considered in the final count
 * 
 * params: none
 * 
 * return: int
 * - normalizd input value with the most instances
 */
int normalizeAnalogInputs() {
  // convert analog inputs to their closest midpoint values 
  int normal[8] = {200, 0, 400, 0, 600, 0, 800, 0};
  // a valid analog input should have at least 4 values above 50 to avoid errant analog signals
  int numValidInputs = 0;
  for (int i=0; i < SAMPLE_SIZE; ++i) {
    int current = analogInputs[i];
    for (int j=0; j < 4; ++j) {
      if (current > (normal[j * 2] - 100) && current < (normal[j * 2] + 100)) {
        ++normal[j * 2 + 1];
        ++numValidInputs;
        break;
      }
    }
  }

  // if number of valid inputs is less than 3, disregard input as errant
  if (numValidInputs < 3) return 0;

  // return midpoint with the most occurrences
  int result = 0;
  int largest = 0;
  for (int k=0; k < 4; k++) {
    if (normal[k * 2 + 1] > largest) {
      result = normal[k * 2];
      largest = normal[k * 2 + 1];
    }
  }
  return result;
} // end normalizeAnalogInputs

/*
 * Handle the normalized analog input
 * 
 * params: int
 * input - value of pressed analog button
 * 
 * return: none
 */
void handleAnalogInput(int input) { 
  if (debug) printInterfaceInput(input);
  switch(input) {
    case 200:
      // toggle display on/off
      // black button pressed 3.51kohm
      if (isDisplayOn) {
        selectDisplayOn = false;
        turnOffScreen();
        timer.lcdTimeoutStart = timer.lcdTimeoutStart - LCD_TIMEOUT_INTERVAL;
      } else {
        selectDisplayOn = true;
        turnOnScreen();
      }
      break;
    case 400:
      // change lcd page
      // white button pressed 1.5kohm
      switch(currentPage) {
        case 'm':
          loadLCDPage('c');
          break;
        case 'c':
          loadLCDPage('t');
          break;
        case 't':
          loadLCDPage('r');
          break;
        case 'r':
          loadLCDPage('h');
          break;
        case 'h':
          loadLCDPage('s');
          break;
        case 's':
          loadLCDPage('m');
          break;
        default:
          loadLCDPage('m');
          break;
      }
      break;
    case 600: 
      // increment values or toggle thermowell enabling
      // red button pressed 670ohm
      if (currentPage == 'c' 
          && monitored->target < MAX_TEMP 
          && request.temperature < MAX_TEMP) {
        request.temperature += 0.5;
        request.isPending = true;
        loadLCDPage('c');
      } else if (currentPage == 't' 
                 && monitored->tolerance < MAX_TOLERANCE 
                 && request.tolerance < MAX_TOLERANCE) {
        request.tolerance += 0.5;
        request.isPending = true;
        loadLCDPage('t');
      } else if(currentPage == 'r') {
        request.isCoolingEnabled = !request.isCoolingEnabled;
        request.isPending = true;
        loadLCDPage('r');
      } else if(currentPage == 'h') {
        request.isHeatingEnabled = !request.isHeatingEnabled;
        request.isPending = true;
        loadLCDPage('h');
      } else if (currentPage == 's') {
        request.isThermowellEnabled = !request.isThermowellEnabled;
        request.isPending = true;
        loadLCDPage('s');
      }
      break;
    case 800: 
      // decrement values or toggle thermowell enabling
      // blue button pressed 270ohm
      if (currentPage == 'c' 
          && monitored->target > MIN_TEMP
          && request.temperature > MIN_TEMP) {
        request.temperature -= 0.5;
        request.isPending = true;
        loadLCDPage('c');
      } else if (currentPage == 't' 
                 && monitored->tolerance > MIN_TOLERANCE
                 && request.tolerance > MIN_TOLERANCE) {
        request.tolerance -= 0.5;
        request.isPending = true;
        loadLCDPage('t');
      } else if(currentPage == 'r') {
        request.isCoolingEnabled = !request.isCoolingEnabled;
        request.isPending = true;
        loadLCDPage('r');
      } else if(currentPage == 'h') {
        request.isHeatingEnabled = !request.isHeatingEnabled;
        request.isPending = true;
        loadLCDPage('h');
      } else if (currentPage == 's') {
        request.isThermowellEnabled = !request.isThermowellEnabled;
        request.isPending = true;
        loadLCDPage('s');
      }
      break;
    default:
      break;
  }
  delay(200);
  timer.lcdTimeoutStart = millis();
}

