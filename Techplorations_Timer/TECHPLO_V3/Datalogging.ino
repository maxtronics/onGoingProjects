void initSD(){
  //Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(sdPin)) {
    //Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
}

void saveToSD(String _fileName, String data){
  File dataFile = SD.open(_fileName, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(data);
    dataFile.close();
    // print to the serial port too:
    //Serial.print("SAVING: ");
    //Serial.println(data);
  }
  // if the file isn't open, pop up an error:
  else {
    String errMsg = "error opening " + _fileName;
    //Serial.println(errMsg);
  }
}

void datalogEvent(String engineStatus){
   String dataToSave = getDateAndTime() + " " + engineStatus + " " + String((totalRuntime / unitDivisor), 2) + "Hrs";
   saveToSD("DATALOGS.txt", dataToSave);
}
