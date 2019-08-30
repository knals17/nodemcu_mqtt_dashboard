#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

#include  <Wire.h>
#include  <LiquidCrystal_I2C.h>

WiFiServer server(80);

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//-------------------VARIABLES GLOBALES--------------------------
int contconexion = 0;

//const char *ssid = "HS";
//const char *password = "chivochivo4025";
char   SERVER[50]   = "3.83.223.148"; //"m11.cloudmqtt.com"
int    SERVERPORT   = 16287;
String USERNAME = "nodemcu";
char   PASSWORD[50] = "12345678";

unsigned long previousMillis = 0;

char charPulsador [15];
String strPulsador;
String strPulsadorUltimo;

char PLACA[50];

char valueStr[15];
String strtemp = "";
char TEMPERATURA[50];
char PULSADOR[50];
char SALIDADIGITAL[50];
char SALIDADIGITAL2[50];

//-------------------------------------------------------------------------
WiFiClient espClient;
PubSubClient client(espClient);

//------------------------CALLBACK-----------------------------
void callback(char* topic, byte* payload, unsigned int length) {

  char PAYLOAD[5] = "    ";

  Serial.print("Mensaje Recibido: [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    PAYLOAD[i] = (char)payload[i];
  }
  Serial.println(PAYLOAD);

  if (String(topic) ==  String(SALIDADIGITAL)) {
    if (payload[1] == 'N') {
      digitalWrite(D6, LOW);
    }
    if (payload[1] == 'F') {
      digitalWrite(D6, HIGH);
    }
  }

  if (String(topic) ==  String(SALIDADIGITAL2)) {
    if (payload[1] == 'N') {
      digitalWrite(D7, LOW);
    }
    if (payload[1] == 'F') {
      digitalWrite(D7, HIGH);
    }
  }

  if (String(topic) ==  String(SALIDADIGITAL)) {
    if (payload[0] == 'A') {
      digitalWrite(D4, HIGH);
    }
    if (payload[0] == 'B') {
      digitalWrite(D4, LOW);
    }
  }
  
}

//------------------------RECONNECT-----------------------------
void reconnect() {
  uint8_t retries = 3;
  // Loop hasta que estamos conectados
  while (!client.connected()) {
    Serial.print("Intentando conexion MQTT...");
    // Crea un ID de cliente al azar
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    USERNAME.toCharArray(PLACA, 50);
    if (client.connect("", PLACA, PASSWORD)) {
      Serial.println("conectado");
      client.subscribe(SALIDADIGITAL);
      client.subscribe(SALIDADIGITAL2);
    } else {
      Serial.print("fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intenta nuevamente en 5 segundos");
      // espera 5 segundos antes de reintentar
      delay(5000);
    }
    retries--;
    if (retries == 0) {
      // esperar a que el WDT lo reinicie
      while (1);
    }
  }
}

//------------------------SETUP-----------------------------
void setup() {


  //---------------------------LCD init--------------------------
  //-------------------------------------------------------------
  // Indicar a la libreria que tenemos conectada una pantalla de 16x2
  lcd.begin(16, 4);
  // Mover el cursor a la primera posición de la pantalla (0, 0)
  lcd.home();
  // Imprimir "Hola Mundo" en la primera linea
  lcd.print("NodeMCU");
  // Mover el cursor a la segunda linea (1) primera columna
  lcd.setCursor ( 0, 1 );
  // Imprimir otra cadena en esta posicion
  lcd.print("ESP8266");
  delay(3000);
  lcd.clear();
  //-------------------------------------------------------------
  //-------------------------------------------------------------


  //prepara salidas
  pinMode(D4,OUTPUT);
  digitalWrite(D4,LOW);
  pinMode(D7, OUTPUT); // D7 salida digital
  digitalWrite(D7, HIGH); // analogWrite(pin, value);
  pinMode(D6, OUTPUT); // D6 salida digital2
  digitalWrite(D6, HIGH);

  // Entradas
  pinMode(14, INPUT); // D5

  // Inicia Serial
  Serial.begin(115200);
  Serial.println("");

  // Conexión WIFI

  WiFiManager wifiManager;

  //wifiManager.resetSettings(); //RESET Valores de conexion
  
  wifiManager.autoConnect("Nodemcu AP");
  Serial.println("Conectado!!...)");
  Serial.println(" ");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  server.begin();      // Inicia el NodeMCU en modo Station
  lcd.setCursor(0,3);
  lcd.print(WiFi.localIP());


  /*WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED and contconexion <50) { //Cuenta hasta 50 si no se puede conectar lo cancela
    ++contconexion;
    delay(500);
    Serial.print(".");
    }
    if (contconexion <50) {
      //para usar con ip fija
      IPAddress ip(192,168,1,156);
      IPAddress gateway(192,168,1,1);
      IPAddress subnet(255,255,255,0);
      WiFi.config(ip, gateway, subnet);

      Serial.println("");
      Serial.println("WiFi conectado");
      Serial.println(WiFi.localIP());
    }
    else {
      Serial.println("");
      Serial.println("Error de conexion");
    }*/

  client.setServer(SERVER, SERVERPORT);
  client.setCallback(callback);

  String temperatura = "/" + USERNAME + "/" + "temperatura";
  temperatura.toCharArray(TEMPERATURA, 50);
  /*String pulsador = "/" + USERNAME + "/" + "pulsador";
  pulsador.toCharArray(PULSADOR, 50);*/
  String salidaDigital = "/" + USERNAME + "/" + "salidaDigital";
  salidaDigital.toCharArray(SALIDADIGITAL, 50);
  String salidaDigital2 = "/" + USERNAME + "/" + "salidaDigital2";
  salidaDigital2.toCharArray(SALIDADIGITAL2, 50);

}

//--------------------------LOOP--------------------------------
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 1000) { //envia la temperatura cada 10 segundos
    previousMillis = currentMillis;
    int analog = analogRead(17);
    float temp = analog * 0.322265625;
    strtemp = String(temp, 1); //1 decimal
    strtemp.toCharArray(valueStr, 15);
    Serial.println("Enviando: [" +  String(TEMPERATURA) + "] " + strtemp);
    client.publish(TEMPERATURA, valueStr);
    //==============================================================================
    //lcd.clear();
    lcd.home();
    lcd.print("Temp= ");
    lcd.print(strtemp);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("Boton ");
    if(strPulsador == "presionado"){
      lcd.print("ON");
    }else{
      lcd.print("OFF");
    }
    
    //==============================================================================
  }

  if (digitalRead(D5) == 0) {
    strPulsador = "presionado";
  } else {
    strPulsador = "NO presionado";
  }

  if (strPulsador != strPulsadorUltimo) { //envia el estado del pulsador solamente cuando cambia.
    strPulsadorUltimo = strPulsador;
    strPulsador.toCharArray(valueStr, 15);
    Serial.println("Enviando: [" +  String(PULSADOR) + "] " + strPulsador);
    client.publish(PULSADOR, valueStr);
  }
}
