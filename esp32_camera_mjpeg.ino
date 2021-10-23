#include "stream_library.h"



//#include "src/OV2640.h"
//#include <WiFi.h>
//#include <WebServer.h>
//#include <WiFiClient.h>
//
//
////#define CAMERA_MODEL_AI_THINKER
//
////#include "camera_pins.h"
//
//
//#define SSID1 "perdedores"
//#define PWD1  "somos perdedores 1375"

//motion detection 
#include "my_library.h"
#define CAMERA_MODEL_AI_THINKER
// include the relevant libraries
#include <FS.h>
#include <SPIFFS.h>
//#define timeit(label, code) { uint32_t start = millis(); code; uint32_t duration = millis() - start; eloquent::io::print_all("It took ", duration, " millis for ", label); }
#define timeit(label, code) code;
void bring_resources();
//

OV2640 cam;

WebServer server(80);

const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
                      "Access-Control-Allow-Origin: *\r\n" \
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
const int hdrLen = strlen(HEADER);
const int bdrLen = strlen(BOUNDARY);
const int cntLen = strlen(CTNTTYPE);

 
void handle_jpg_stream(void) //loop infinito esto va a ir a un core
{
  char buf[32]; //creo que aca se guarda la imagen
  int s;        //tamano de la imagen

  WiFiClient client = server.client();

  client.write(HEADER, hdrLen);
  client.write(BOUNDARY, bdrLen);
  
  while (true)
  {
    if (!client.connected()) break;

    cam.run(); //saca una nueva foto, el fb queda guardado en la variable de clase
    s = cam.getSize();
    client.write(CTNTTYPE, cntLen);
    sprintf( buf, "%d\r\n\r\n", s );
    client.write(buf, strlen(buf));
    client.write((char *)cam.getfb(), s);
    client.write(BOUNDARY, bdrLen);
    Serial.print("stream en core :");
    Serial.println(xPortGetCoreID());//core en el que se ejecuta
  }
}

const char JHEADER[] = "HTTP/1.1 200 OK\r\n" \
                       "Content-disposition: inline; filename=capture.jpg\r\n" \
                       "Content-type: image/jpeg\r\n\r\n";
const int jhdLen = strlen(JHEADER);
//////////////////////////////////
void handle_jpg(void)
{
  WiFiClient client = server.client();

  cam.run();
  if (!client.connected()) return;

  client.write(JHEADER, jhdLen);
  client.write((char *)cam.getfb(), cam.getSize());
}
/////////////////////////////////
void handleNotFound()
{
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(200, "text / plain", message);
}
//////////////////////SETUP////////////////////////////////////////////////

void setup()
{

  Serial.begin(115200);

  SPIFFS.begin(true);
  delay(1000);

  camera_config_t config;
  set_camara_config(config);
  cam.init(config);
  set_motion_config();
  //start_camera(); //declarada en cpp   
  //set_motion_config();

  IPAddress ip;

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID1, PWD1);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
  }
  ip = WiFi.localIP();//local IP del server
  Serial.println(F("WiFi connected"));
  Serial.println("");
  Serial.println(ip);
  Serial.print("Stream Link: http://");
  Serial.print(ip);
  Serial.println("/mjpeg/1");

  server.on("/mjpeg/1", HTTP_GET, handle_jpg_stream);
  server.on("/jpg", HTTP_GET, handle_jpg);
  server.onNotFound(handleNotFound);
  server.begin();
}
//////////////////////////////MAIN LOOP/////////////////////////////////////////////////
bool iniciar=true;
int count=0;
void loop()
{
  Serial.print("el main se ejecuta en el core :");
  Serial.println(xPortGetCoreID());
  server.handleClient();
  
  if (iniciar and count>=400) {
      iniciar=false;
      Serial.println("entro al iniciador");
      
      delay(5000);    

      esp_camera_deinit();
      camera_config_t config;
      set_camara_config2(config);
       Serial.println("incia la camara");
      cam.init(config);
      
     
     
      
  }  while (!iniciar) {
      
      cam.run();//guarda la foto en el buffer
     
      procces_buffer(cam.getfb()); //camputra frames
      hay_movimiento(); //detecta si hay o no movimiento  
      delay(30);//fps estaba en 30
    }
  count=count+1;
}
