
/* Conversions ASCII to Decimal
 *  -----------
 *  Num | ASCII
 *  -----------
 *    0 |  48
 *    1 |  49 
 *    2 |  50
 *    3 |  51
 *    4 |  52
 *    5 |  53
 *    6 |  54
 *    7 |  55
 *    8 |  56
 *    9 |  57
 *    . |  46 
 */


void saveToEEPROM(String num){
  char toSave[20];
  num = num + "&";
  num.toCharArray(toSave, 21);
  Serial.print("SAVING: "); Serial.println(num);
  Wire.beginTransmission(EEPROM_I2C_ADDRESS); // transmit to EEPROM device
  Wire.write(byte(0x01));
  Wire.write(byte(0x01));  
  for(x = 0; x <= 20; x++){    
    Wire.write(toSave[x]);     
  }    
  Wire.endTransmission();    // stop transmitting
  
}

String readFromEEPROM(){
  
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write(byte(0x01));
  Wire.write(byte(0x01));   
  Wire.endTransmission();
  Wire.requestFrom(EEPROM_I2C_ADDRESS, 20);
  
  String data = "";
  for(x = 0; x< 20; x++){
    byte num = Wire.read();
    if(num == 38){
      break;
    }
    data = data + ASCIItoChar(num);
  }
  
  return data; 
}

String ASCIItoChar(byte num){
  String c;
  switch(num){
    case 48:
      c = '0';
      break;
    case 49:
      c = '1';
      break;
    case 50:
      c = '2';
      break;
    case 51:
      c = '3';
      break;
    case 52:
      c = '4';
      break;
    case 53:
      c = '5';
      break;
    case 54:
      c = '6';
      break;
    case 55:
      c = '7';
      break;
    case 56:
      c = '8';
      break;
    case 57:
      c = '9';
      break;
    case 46:
      c = '.';
      break;
    case 38:
      c = "&";
  }
     return c;
}

void saveStartTimeToEEPROM(String num){
  char toSave[20];
  num = num + "&";
 // Serial.print("SAVING: "); Serial.println(num);
  num.toCharArray(toSave, 21);
  
  Wire.beginTransmission(EEPROM_I2C_ADDRESS); // transmit to EEPROM device
  Wire.write(byte(0x30));
  Wire.write(byte(0x01));  
  for(x = 0; x <= 20; x++){    
    Wire.write(toSave[x]);     
  }    
  Wire.endTransmission();    // stop transmitting
  
}

String readStartTimeEEPROM(){
  
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write(byte(0x30));
  Wire.write(byte(0x01));   
  Wire.endTransmission();
  Wire.requestFrom(EEPROM_I2C_ADDRESS, 20);
  
  String data = "";
  for(x = 0; x< 20; x++){
    byte num = Wire.read();
    if(num == 38){
      break;
    }
    data = data + ASCIItoChar(num);
  }
  
  return data; 
}
