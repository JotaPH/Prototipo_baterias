extern unsigned long tiempoAnterior;
extern float voltajeMinimo;
extern float corrienteMinima;
extern float voltajePrecarga;
extern float corrientePrecarga;
extern float m_n1;
extern float m_n1_2;

extern bool cargarBateria;
extern bool descargarBateria;

extern int estadoActualC;
extern int estadoActualD;
extern int estadoActualR;

bool hacerPrecarga = false;

void leerSerial() {

  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando.length() < 2) return;

    else if (comando.length() == 2 && comando[0] == 'E'){
      empezar = comando.substring(1, 2).toInt();
      prepararCiclo();
      tiempoAnterior = millis();
    }

    else if (comando.length() == 10 && comando[0] == 'R'){
      
      estadoActualTexto = "Midiendo RInterna";
      estadoActualR = 0;
      SetPoint = comando.substring(3).toFloat();
      medirR = true;
    }

    else if (comando.length() == 51 && comando[0] == 'A'){
      //A01,1500.00,100.00,2900.00,100.00,2900.00,0,9999.99
      
      cargarBateria = comando.substring(1, 2).toInt();
      descargarBateria = comando.substring(2, 3).toInt();
      SetPoint = comando.substring(4, 11).toFloat();
      corrienteMinima = comando.substring(12, 18).toFloat();
      voltajeMinimo = comando.substring(19, 26).toFloat();
      corrientePrecarga = comando.substring(27, 33).toFloat();
      voltajePrecarga = comando.substring(34, 41).toFloat();
      hacerPrecarga = comando.substring(42, 43).toInt();
      capacidadConocida = comando.substring(44).toInt();
      empezar = false;

      prepararCiclo();

      if (descargarBateria){
        estadoActualTexto = "Esperando descarga";
        calibracionCarga = 0.0468;
      }

      else if (cargarBateria){
        estadoActualTexto = "Esperando carga";
      }

      else estadoActualTexto = "En reposo";
    }

    else Serial.println("\nERROR: Comando no valido");

  }
}

void prepararCiclo(){
    
  estadoActualC = hacerPrecarga ? 1 : 2;
  estadoActualD = 1;

  ledcWrite(controlDescarga, 0);
  ledcWrite(controlCarga, 0);

  digitalWrite(habilitarDescarga, 1);
  digitalWrite(habilitarCarga, 0);

  m_n1 = 0.0;
  m_n1_2 = 0.0;

  if (empezar == 0) estadoActualSistema = "Pausado";
  else if (empezar == 1) estadoActualSistema = "Iniciado";
  else{
    estadoActualSistema = "Reiniciado";
    tiempoTranscurrido = 0.0;
    capacidad_mAh = 0.0;
    medirR = false;
  }
}