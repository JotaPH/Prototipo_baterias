#include <Wire.h>
#include <Adafruit_INA3221.h>
#include <math.h>
#include <string.h>

Adafruit_INA3221 ina3221;

#define controlCarga 17
#define habilitarCarga 19
#define habilitarDescarga 26
#define controlDescarga 32
#define habilitarBateria 33
#define detectarBateria 34
#define ledIndicador 2

// Salidas activas: relés HIGH; habilitación de descarga LOW.
// Los PWM siempre arrancan en cero.
enum EstadoSistema {
  IDLE, PAUSADO, PRECARGA, CARGA_CC, CARGA_CV, DESCARGA,
  RINT_ESTABILIZAR_I1, RINT_MUESTREAR_I1,
  RINT_ESTABILIZAR_I2, RINT_MUESTREAR_I2,
  FINALIZADO, FALLO
};
enum ModoOperacion { MODO_NINGUNO, MODO_CARGA, MODO_DESCARGA };

EstadoSistema estado = IDLE;
EstadoSistema estadoReanudacion = IDLE;
ModoOperacion modoOperacion = MODO_NINGUNO;

float SetPoint = 500.0;
float corrienteMinima = 100.0;
float voltajeMinimo = 2900.0;
float corrientePrecarga = 100.0;
float voltajePrecarga = 2900.0;
bool permitirPrecargaBajo2V = false;
bool capacidadConocida = false;
float capacidadConocida_mAh = 0.0;
bool configuracionRecibida = false;
bool modoRecibido = false;

float corrienteBateria = 0.0;
float voltajeBateria = 0.0;
float temperaturaBateria = 0.0;
float calibracionCarga = 0.0;
float capacidadCarga_mAh = 0.0;
float capacidadDescarga_mAh = 0.0;
float energiaCarga_mWh = 0.0;
float energiaDescarga_mWh = 0.0;
float tiempoCarga_s = 0.0;
float tiempoDescarga_s = 0.0;
float RInterna = 0.0;
bool RInternaValida = false;
bool bateriaValida = false;

float I1_R = 200.0, I2_R = 1000.0;
uint32_t t1_R_ms = 10000, t2_R_ms = 2000;
uint32_t idPruebaR = 0;
bool rintSolicitada = false;
bool rintEnCurso = false;
float sumaI_R = 0.0, sumaV_R = 0.0;
uint32_t muestrasR = 0;
float promedioI1 = 0.0, promedioV1 = 0.0;
float promedioI2 = 0.0, promedioV2 = 0.0;
uint32_t inicioMuestreoR = 0;
uint8_t muestrasEstablesR = 0;
float toleranciaR_mA = 5.0;

String motivoPausa = "";
String estadoActualTexto = "En reposo";
String estadoActualSistema = "IDLE";

uint32_t tiempoLoopAnterior = 0;
uint32_t tiempoMedicionAnterior = 0;
uint32_t tiempoTelemetriaAnterior = 0;
uint32_t tiempoSeguridadAnterior = 0;
uint32_t tiempoCargaAnterior = 0;
uint32_t tiempoDescargaAnterior = 0;
uint32_t tiempoAcumuladoresAnterior = 0;
uint32_t muestrasSobreVoltaje = 0;
uint32_t muestrasSobreCorriente = 0;
uint32_t muestrasSobreTemperatura = 0;
bool alarmaActiva = false;

bool cargarBateria = false;
bool descargarBateria = false;
bool medirR = false;
int empezar = 0;
float Ts = 0.1f;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(20);

  // Establecer primero los niveles inactivos para evitar pulsos al arrancar.
  pinMode(habilitarBateria, OUTPUT);
  pinMode(habilitarCarga, OUTPUT);
  pinMode(habilitarDescarga, OUTPUT);
  pinMode(detectarBateria, INPUT);
  pinMode(ledIndicador, OUTPUT);
  digitalWrite(habilitarBateria, LOW);
  digitalWrite(habilitarCarga, LOW);
  digitalWrite(habilitarDescarga, HIGH);
  digitalWrite(ledIndicador, LOW);

  ledcAttach(controlDescarga, 5000, 12);
  ledcAttach(controlCarga, 10000, 12);
  ledcWrite(controlDescarga, 0);
  ledcWrite(controlCarga, 0);

  Wire.begin(21, 22);
  if (!ina3221.begin()) {
    Serial.println("ERROR: INA3221 no encontrado. Salidas permanecen deshabilitadas.");
    while (true) {
      digitalWrite(habilitarCarga, LOW);
      digitalWrite(habilitarDescarga, HIGH);
      digitalWrite(habilitarBateria, LOW);
      ledcWrite(controlCarga, 0);
      ledcWrite(controlDescarga, 0);
      delay(100);
    }
  }

  resetPID_Carga();
  resetPID_Descarga();
  tiempoLoopAnterior = millis();
  tiempoMedicionAnterior = millis();
  tiempoTelemetriaAnterior = millis();
  tiempoSeguridadAnterior = millis();
  tiempoAcumuladoresAnterior = millis();
  emitirEvento("SISTEMA_LISTO", "");
}

void loop() {
  leerSerial();
  uint32_t ahora = millis();

  if (ahora - tiempoMedicionAnterior >= 50) {
    medirParamtros();
    tiempoMedicionAnterior = ahora;
    actualizarReleBateria();
  }

  if (ahora - tiempoSeguridadAnterior >= 100) {
    revisarSeguridad();
    tiempoSeguridadAnterior = ahora;
  }

  if (ahora - tiempoLoopAnterior >= 100) {
    float dt = (ahora - tiempoAcumuladoresAnterior) / 1000.0f;
    actualizarAcumuladores(dt);
    tiempoAcumuladoresAnterior = ahora;
    ejecutarMaquinaEstados();
    tiempoLoopAnterior = ahora;
  }

  if (ahora - tiempoTelemetriaAnterior >= 500) {
    enviar_Datos();
    tiempoTelemetriaAnterior = ahora;
  }
}
