void wifi_start(){
  Serial.println("Starting search of known WiFi.."); 
  WiFi.mode(WIFI_STA);
  int time_to_connect = 10000;
  unsigned long start_time_to_connect = millis(); 
  int numberOfNetworks = WiFi.scanNetworks();
  if (numberOfNetworks>0) {                         //Если найдены SSID
          for(int i =0; i<numberOfNetworks; i++){    //Перебираем все имена SSID
            for (int k = 0; k < (count_ssid); k++){   //Перебираем все имена SSID в массиве      
              if (WiFi.SSID(i)==ssid[k]){
                  Serial.print(WiFi.SSID(i));
                  Serial.print("->");
                  Serial.println((ssid[k])); 
                  WiFi.begin(ssid[k], password[k]);
                  while (millis()<(start_time_to_connect+time_to_connect) && (WiFi.status() != WL_CONNECTED)){ //пытаемся подключиться в течении времени                
                    delay(100);
                  }
               if (WiFi.status() == WL_CONNECTED) {
                 Serial.println("Connected!");  break;}  
              }
            }   
          }
  } else
  {
    Serial.println("nothing found..");
  }  
  if (WiFi.status() != WL_CONNECTED){} 
}
