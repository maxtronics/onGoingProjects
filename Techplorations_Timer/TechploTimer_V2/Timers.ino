
void getRTCVals(){
  readDS3231time(&g_sec, &g_min, &g_hour, &g_dayOfWeek, &g_day, &g_month, &g_year);
}


unsigned long int getEpochTime(){
  getRTCVals();
  setTime(g_hour,g_min,g_sec,g_day,g_month,g_year);                    //re-init Timer time to current Time based on RTC's time
  unsigned long int currentEpochTime = now();                    //get current time in EPOCH unit.
  return currentEpochTime;
}

String getDate(){
  getRTCVals();
  String _date;
  _date = String(g_month) + "/" +  String(g_day) + "/" + String(g_year);
  return _date;
}

String getTime(){
  getRTCVals();
  String _time;
  _time = String(g_hour) + ":" +  String(g_min) + ":" + String(g_sec);
  return _time;
}

String getDateAndTime(){
  String data = getDate() + " " + getTime();
  return data;
}
