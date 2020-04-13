void engineOn() {
    if(STATE == 1){
      STATE = 5;
      isExecuted = false;
      Serial.println("STATE = 5");
      Serial.println("ENGINE ON!");
    }

    if(STATE == 3){
      STATE = 4;
      isExecuted = false;
      Serial.println("STATE = 4");
      Serial.println("ENGINE ON!");
    }
    
    
 
}

void engineOff() {

    if(STATE == 5 || STATE == 4){
      STATE = 3;
      isExecuted = false;
      //Serial.println("STATE = 3");
      //Serial.println("ENGINE OFF!");
    }
    

}
