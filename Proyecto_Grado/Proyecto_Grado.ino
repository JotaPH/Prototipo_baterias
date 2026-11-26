#include <Wire.h>
#include <Adafruit_INA3221.h>

Adafruit_INA3221 ina3221;

#define controlCarga 17        //MOSFET CANAL P EN CARGA
#define habilitarCarga 19      //RELÉ EN CARGA
#define habilitarDescarga 26   //MOSFET CANAL N EN DESCARGA
#define controlDescarga 32     //MOSFET CANAL N EN DESCARGA
#define habilitarBateria 33    //RELÉ EN BATERIA
#define detectarBateria 34     //ENTRADA DE VOLTAJE POSITIVO
#define ledIndicador 2         //LED Y RELÉ DE BATERIA VÁLIDA

unsigned long tiempo_inicio = 0;
unsigned long tiempo_datos = 0;

extern float Ts;
extern bool cargarBateria;
extern bool descargarBateria;

float tiempoTranscurrido = 0.0;
float temperaturaBateria = 0.0;
float capacidadConocida = 0.0;
float corrienteBateria = 0.0;
float calibracionCarga = 0.0;
float voltajeBateria = 0.0;
float capacidad_mAh = 0.0;
float RInterna = 0.0;
float SetPoint = 0.0;

bool bateriaValida = false;
bool medirR = false;

int empezar = 0;

String estadoActualTexto = "En reposo";
String estadoActualSistema = "Pausado";

void setup() {

  Serial.begin(115200);

  pinMode(habilitarBateria, OUTPUT);
  pinMode(habilitarCarga, OUTPUT);
  pinMode(habilitarDescarga, OUTPUT);
  pinMode(detectarBateria, INPUT);
  pinMode(ledIndicador, OUTPUT);

  ledcAttach(controlDescarga, 5000, 12);
  ledcAttach(controlCarga, 10000, 12);

  ledcWrite(controlDescarga, 0);
  ledcWrite(controlCarga, 0);

  digitalWrite(habilitarBateria, 0);
  digitalWrite(habilitarCarga, 0);
  digitalWrite(habilitarDescarga, 1);

  delay(150);

  Wire.begin(21, 22);

  if (!ina3221.begin()) {
    Serial.println("ERROR: INA3221 no encontrado.");
    while (1);
  }

  tiempo_inicio = millis();
  tiempo_datos = millis();
}

void loop() {

  leerSerial();
  medirParamtros();
  if (medirR) medirRInterna();

  if(millis() - tiempo_inicio > Ts * 1000){

    if (cargarBateria && empezar == 1) secuencia_Carga();
    else if (descargarBateria && empezar == 1) secuencia_Descarga();
    
    tiempo_inicio = millis();
  }

  if(millis() - tiempo_datos > 500) {
    enviar_Datos();
    revisarBateria();
    tiempo_datos = millis();
  }
}