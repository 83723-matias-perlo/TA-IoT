#include <WiFi.h>

#include <Adafruit_SSD1306.h>
#include <splash.h>
#include "config.h"


WiFiServer server(80);
Adafruit_SSD1306 display = Adafruit_SSD1306(128,32, &Wire);

const char* ssid = "Labsis-Cursos";
const char* password = "";

String header;       // Variable para guardar el HTTP request
const int PIN_LED = 2 ;

//------------------------CODIGO HTML------------------------------
String pagina =   "<!DOCTYPE html>"
                  "<head>"
                  "<meta charset='UTF-8'>"
                  "<title>Document</title>"
                  "</head>"
                  "<body style='text-align: center;'>"
                  "<h1>IoT Fantastico!</h1>"
                  "<div>"
                  "<p>LED = %0</p>"
                  "<div>"
                  "<input type='range' id='led' onchange='console.log(this.value)'>"
                  "</div>"
                  "</div>"
                  "</body>"
                  "</html>";


void displayInit(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("Bienvenido!"));
  display.printf("Fecha %s\n",__DATE__);
  display.printf("Hora: %s\n",__TIME__);
  display.display();
  delay(1000);

}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Conexión WIFI
  WiFi.begin(ssid, password);
  //Cuenta 500 milisegundos para preguntar nuevamente si se pudo conectar
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());
  server.begin();  // iniciamos el servidor


  
  // Para pantalla
  displayInit();
  display.setCursor(0,0);
}

void loop() {
  WiFiClient client = server.available();  // Escucha a los clientes entrantes. Verifica si hay alguna solicitud de conexión pendiente

  if (client) {                     // Si se conecta un nuevo cliente

    String currentLine = "";        //
    
    while (client.connected()) {    // loop mientras el cliente está conectado
      if (client.available()) {     // si hay bytes para leer desde el cliente. Devuelve el número de bytes disponibles para lectura en el búfer de entrada del cliente
        char c = client.read();     // lee un byte y lo elimina del buffer
        header += c;
        if (c == '\n') {  // si el byte es un caracter de salto de linea
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");        //la solicitud HTTP se ha procesado correctamente.
            client.println("Content-type:text/html"); //establece el tipo de contenido que se enviará al cliente en la respuesta. En este caso se trata de una página HTML.
            client.println("Connection: close");      //la conexión entre el servidor y el cliente se cerrará después de enviar la respuesta
            client.println();

            if (header.indexOf("msg=") >= 0){
              // cargarlo al LCD
              String mensaje = "";
              for (int i=header.indexOf("GET /msg=")+4; i<header.length();i++){ // corroborar
                if(header[i]=='%'){
                  i+=3;
                }
                mensaje +=header[i];
              }
              display.clearDisplay();
              display.setCursor(0,0);
              display.println(mensaje); // carga mensaje al LCD
              display.display();
            }

            if (header.indexOf("led=") >= 0) {   //indica si se setea un valor al led
              String valor_led = "";
              for (int i=header.indexOf("GET /led=")+4; i<header.length();i++){ // corroborar
                if(header[i]=='&'){
                  break;
                }
                valor_led +=header[i];
              }
              int valor_int = valor_led.toInt();
              analogWrite(PIN_LED, map(valor_int,0,100,0,255));
            } 

            // Muestra la página web
            client.println(pagina);

           // la respuesta HTTP temina con una linea en blanco
            client.println();
            break;
          } else {  // si tenemos una nueva linea limpiamos currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // si C es distinto al caracter de retorno de carro
          currentLine += c;      // lo agrega al final de currentLine
        }
      }
    }
    // Limpiamos la variable header
    header = "";
    // Cerramos la conexión
    client.stop();
  }
}