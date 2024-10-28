\diagram{#include "DHT.h"

#define umidadeAnalogica A0 //Atribui o pino A0 a variável umidade - leitura analógica do sensor
#define umidadeDigital 7 //Atribui o pino 7 a variável umidadeDigital - leitura digital do sensor
#define LedVermelho 5 //Atribui o pino 7 a variável LedVermelho
#define LedVerde 6 //Atribui o pino 6 a variável LedVerde
#define DHTPIN A4     // Pin donde está conectado el sensor
#define DHTTYPE DHT11   // Cambia a DHT22 si usas un sensor DHT22
int nRainIn = A2;
int nRainDigitalIn = 2;
int nRainVal;
boolean bIsRaining = false;
String strRaining;

DHT dht(DHTPIN, DHTTYPE);

int valorumidade; //Declaração da variável que armazenará o valor da umidade lida - saída analogica
int valorumidadeDigital; //Declaração da variável que armazenara a saída digital do sensor de umidade do solo

void setup() {
  Serial.begin(9600); //Incia a comunicação serial
  pinMode(umidadeAnalogica, INPUT); //Define umidadeAnalogica como entrada
  pinMode(umidadeDigital, INPUT); //Define umidadeDigital como entrada
  pinMode(LedVermelho, OUTPUT); //Define LedVermelho como saída
  pinMode(LedVerde, OUTPUT); //Define LedVerde como saída
  dht.begin();
  pinMode(2,INPUT);
  pinMode(3,OUTPUT);
  digitalWrite(3,LOW);
}

void loop() {

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  float h = dht.readHumidity();
  // Leer la temperatura en Celsius
  float t = dht.readTemperature();
  // Leer la temperatura en Fahrenheit
  float f = dht.readTemperature(true);

  // Verificar si alguna lectura falló e imprimir un error
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Error al leer el sensor DHT");
    return;
  }

  // Calcular el índice de calor en Fahrenheit
  float hif = dht.computeHeatIndex(f, h);
  // Calcular el índice de calor en Celsius
  float hic = dht.computeHeatIndex(t, h, false);

  // Imprimir los valores en el Serial Monitor
  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(t);

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
  //PARTE DO SENSOR DE CHUVA
  nRainVal = analogRead(nRainIn);
  bIsRaining = !(digitalRead(nRainDigitalIn));
  
  if(bIsRaining){
    strRaining = "YES";
  }
  else{
    strRaining = "NO";
  }
  
  Serial.print("Raining?: ");
  Serial.print(strRaining);  
  Serial.print("t Moisture Level: ");
  Serial.println(nRainVal);

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  //PARTE DO SENSOR UMIDADE DO SOLO
  valorumidade = analogRead(umidadeAnalogica); //Realiza a leitura analógica do sensor e armazena em valorumidade
  valorumidade = map(valorumidade, 1023, 315, 0, 100); //Transforma os valores analógicos em uma escala de 0 a 100
  Serial.print("Umidade encontrada: "); //Imprime mensagem
  Serial.print(valorumidade); //Imprime no monitor serial o valor de umidade em porcentagem
  Serial.println(" % " );

  if (valorumidade < 50){
    digitalWrite(3,HIGH);
  }
  else{
    digitalWrite(3,LOW);
  }

  valorumidadeDigital = digitalRead(umidadeDigital); //Realiza a leitura digital do sensor e armazena em valorumidadeDigital
  if (valorumidadeDigital == 0) { //Se esse valor for igual a 0, será mostrado no monitor serial que o solo está úmido e o led verde se acende
    Serial.println("Status: Solo úmido");
    digitalWrite(LedVermelho, LOW);
    digitalWrite(LedVerde, HIGH);
  }
  else { // se esse valor for igual a 1, será mostrado no monitor serial que o solo está seco e o led vermelho se acende
    Serial.println("Status: Solo seco");
    digitalWrite(LedVermelho, HIGH);
    digitalWrite(LedVerde, LOW);
  }
  delay(1500); //Atraso de 500ms
}}

#include "WiFiS3.h"
// Definições para o sensor de umidade do solo
#define soilWet 500   // Valor analógico considerado 'solo molhado'
#define soilDry 750   // Valor analógico considerado 'solo seco'
#define sensorPower 7  // Pino de alimentação do sensor
#define sensorPin A0   // Pino analógico onde o sensor está conectado
#define waterpump 3 // Pino digital de liga desliga da bomba

// WiFi
char ssid[] = "wPEM";       // your network SSID (name)
char pass[] = "ufrjmecanica";   // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
WiFiServer server(80);

// Variáveis de exemplo para outros sensores
bool rainDetected = false;   // Simula sensor de chuva (substitua por leitura real, se disponível)
float temperature = 26.5;    // Simula leitura de temperatura
int humidity = 60;           // Simula leitura de umidade do ar
int soilMoisture = 0;        // Variável para armazenar a leitura do sensor de umidade do solo

void setup() {
  Serial.begin(9600);
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);  // Inicialmente mantenha o sensor desligado
  analogWrite(waterpump, 33);

  // Tentativa de conexão WiFi
  while (status != WL_CONNECTED) {
    Serial.print("Tentando conectar a rede: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  
  server.begin();  // Inicia o servidor
  printWifiStatus();  // Mostra o status da conexão
}

void loop() {
  // Atualiza o valor da umidade do solo
  int moisture = readSoilMoisture();
  Serial.print("Umidade do solo: ");
  Serial.println(moisture);
  // Determine status of our soil
  if (moisture < soilWet) {
    Serial.println("Status: Soil is too wet");
  } else if (moisture >= soilWet && moisture < soilDry) {
    Serial.println("Status: Soil moisture is perfect");
  } else {
    Serial.println("Status: Soil is too dry - time to water!");
    analogWrite(waterpump, 0);
  }
  delay(1000);  // Take a reading every second for testing

  WiFiClient client = server.available();  // Espera por clientes

  if (client) {  // Se houver um cliente conectado
    Serial.println("Novo cliente conectado");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);  // Debugging no monitor serial
        
        if (c == '\n' && currentLine.length() == 0) {
          // Envia a resposta HTTP (headers e conteúdo HTML)
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println();

          // Verifica se a requisição foi para "/sensor-data" e retorna JSON
          if (currentLine.endsWith("GET /sensor-data")) {
            // Envia dados dos sensores no formato JSON
            client.println("{");
            client.print("\"rain\":");
            client.print(rainDetected ? "true" : "false");
            client.println(",");
            client.print("\"soilMoisture\":");
            client.print(moisture);  // Envia o valor real de umidade do solo
            client.println(",");
            client.print("\"temperature\":");
            client.print(temperature);
            client.println(",");
            client.print("\"humidity\":");
            client.print(humidity);
            client.println("}");
          } else {
            // Caso contrário, retorna a página HTML com os sensores
            client.println("<!DOCTYPE html><html lang='en'><head>");
            client.println("<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>");
            client.println("<title>Weather Station Dashboard</title>");
            client.println("<style>body{font-family:Arial,sans-serif;background-color:#f4f4f9;text-align:center;padding:20px;}");
            client.println("h1{color:#333;}.sensor-data{display:flex;justify-content:space-around;flex-wrap:wrap;margin-top:20px;}");
            client.println(".sensor{background-color:#fff;border-radius:10px;box-shadow:0 0 10px rgba(0,0,0,0.1);width:200px;padding:20px;margin:10px;}");
            client.println(".sensor h2{font-size:1.2em;color:#444;}.sensor p{font-size:1.5em;color:#000;}</style></head>");
            client.println("<body><h1>Weather Station Data</h1><div class='sensor-data'>");
            client.println("<div class='sensor'><h2>Rain Sensor</h2><p id='rain'>--</p></div>");
            client.println("<div class='sensor'><h2>Soil Moisture</h2><p id='moisture'>" + String(moisture) + "</p></div>");
            client.println("<div class='sensor'><h2>Temperature</h2><p id='temperature'>--</p></div>");
            client.println("<div class='sensor'><h2>Humidity</h2><p id='humidity'>--</p></div></div>");
            
            // Script para atualizar os dados em tempo real usando AJAX
            client.println("<script>function updateSensorData(){");
            client.println("fetch('/sensor-data').then(response=>response.json()).then(data=>{");
            client.println("document.getElementById('rain').innerText=data.rain?'Rain Detected':'No Rain';");
            client.println("document.getElementById('moisture').innerText=data.soilMoisture+'%';");
            client.println("document.getElementById('temperature').innerText=data.temperature+'°C';");
            client.println("document.getElementById('humidity').innerText=data.humidity+'%';");
            client.println("}).catch(error=>console.error('Error fetching sensor data:',error));}");
            
            // Atualiza os dados a cada 5 segundos
            client.println("setInterval(updateSensorData,5000);");
            client.println("updateSensorData();");
            client.println("</script></body></html>");
          }
          break;
        }
        if (c == '\n') {
          currentLine = "";
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();  // Fecha a conexão com o cliente
    Serial.println("Cliente desconectado");
  }
 
}

// Função para ler a umidade do solo e mapear para uma escala de 0 a 100%
int readSoilMoisture() {
  digitalWrite(sensorPower, HIGH);  // Liga o sensor
  delay(10);  // Aguarda o sensor estabilizar
  int val = analogRead(sensorPin);  // Lê o valor analógico do sensor
  digitalWrite(sensorPower, LOW);  // Desliga o sensor
  return val;  // Retorna o valor lido
}

void printWifiStatus() {
  // Exibe o SSID da rede conectada
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Exibe o endereço IP da placa
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Exibe a força do sinal
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  // Exibe o endereço para acessar no navegador
  Serial.print("Para acessar a página, abra o navegador em http://");
  Serial.println(ip);
}

