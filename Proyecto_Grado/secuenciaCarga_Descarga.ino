bool cargarBateria = 0;
bool descargarBateria = 0;

int estadoActualC = 0;
int estadoActualD = 0;
int estadoActualR = 0;

float corrienteMinima = 100.0;
float voltajeMinimo = 2900.00;
float corrientePrecarga = 100.0;
float voltajePrecarga = 2900.0;

float V1 = 0.0;
float V2 = 0.0;
float V_muestras[10] = {0.0};
float v_promedio = 0.0;

unsigned long tiempoAnterior = 0;
unsigned long tiempoActual = 0;
unsigned long tiempoRInterna = 0;

extern float m_n1_2;
extern float m_n1;

void secuencia_Carga(){

  switch(estadoActualC){
    case 0:
      empezar = 0;
      m_n1_2 = 0;
      estadoActualTexto = "Carga finalizada";
      ledcWrite(controlCarga, 0);
      digitalWrite(habilitarCarga, 0);
      break;
    
    case 1:
      estadoActualTexto = "Precarga";
      SetPoint = corrientePrecarga;
      digitalWrite(habilitarCarga, 1);
      calcularPID_Carga(false);
      if (voltajeBateria >= voltajePrecarga) estadoActualC = 2;
      break;

    case 2:
      estadoActualTexto = "Cargando en CC";
      digitalWrite(habilitarCarga, 1);
      calcularPID_Carga(false);
      if (voltajeBateria >= 4200.0) estadoActualC = 3;
      break;

    case 3:
      estadoActualTexto = "Cargando en CV";
      calcularPID_Carga(true);
      if (corrienteBateria <= corrienteMinima) estadoActualC = 0;
      break;
  }
}

void secuencia_Descarga(){

  switch (estadoActualD) {
    case 0: 
      empezar = 0;
      m_n1 = 0.0;
      estadoActualTexto = "Descarga finalizada";
      ledcWrite(controlDescarga, 0);
      digitalWrite(habilitarDescarga, 1);
      break;

    case 1:
      estadoActualTexto = "Descargando";
      digitalWrite(habilitarDescarga, 0);
      calcularPID_Descarga();
      conteoCapacidad();
      if (voltajeBateria <= voltajeMinimo)  estadoActualD = 0;
      break;
  }
}

void medirRInterna(){
  
  switch(estadoActualR){
    case 0:
      if (capacidad_mAh <= capacidadConocida / 2.0){
        estadoActualD = 1;
        secuencia_Descarga();
      }
      else{
        estadoActualR = 1;
        memset(V_muestras, 0, sizeof(V_muestras));
        tiempoRInterna = millis();
      }
      break;
    
    case 1:
      SetPoint = 200.0;
      estadoActualD = 1;
      secuencia_Descarga();

      v_promedio = 0.0;

      for(int i=9; i>=0; i--){
        if (i == 0) V_muestras[i] = voltajeBateria;
        else V_muestras[i] = V_muestras[i-1];
        v_promedio += V_muestras[i];
      }

      V1 = v_promedio/10.0;

      if (millis() - tiempoRInterna >= 10000) estadoActualR = 2;
      break;
    
    case 2:
      SetPoint = 1000.0;
      estadoActualD = 1;
      secuencia_Descarga();

      V2 = voltajeBateria;

      if (abs(corrienteBateria) >= 1000.0){
        RInterna = (V1 - V2) / (abs(corrienteBateria) - 200.0);
        estadoActualR = 3;
      }
      break;
    
    case 3:
      medirR = false;
      prepararCiclo();
      estadoActualTexto = "RInterna medida";
  }
}

void conteoCapacidad(){

  tiempoActual = millis();
  float dt = (tiempoActual - tiempoAnterior) / 1000.0;
  tiempoTranscurrido += dt;
  tiempoAnterior = tiempoActual;

  float corriente_mA = abs(corrienteBateria);
  capacidad_mAh += (corriente_mA * dt) / 3600.0;
}