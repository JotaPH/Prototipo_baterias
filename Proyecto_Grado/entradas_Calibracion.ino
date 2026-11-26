void medirParamtros(){

  float voltajeADC = (analogRead(detectarBateria)*3300.0)/4095.0; 

  if (!bateriaValida && voltajeADC >= 10.0){
    bateriaValida = true;
    digitalWrite(ledIndicador, 1);
  }

  else if (bateriaValida && voltajeADC <= 1.0){
    bateriaValida = false;
    digitalWrite(habilitarBateria, 0);
    digitalWrite(ledIndicador, 0);
  }

  corrienteBateria = ina3221.getCurrentAmps(0)*1000.0;

  voltajeBateria = ina3221.getBusVoltage(1)*1000.0;
  calibrarVoltajeCarga();

  temperaturaBateria = ina3221.getBusVoltage(2)*1000.0 + 6.0;
  calcularTemperatura();
}

void revisarBateria(){
  if (bateriaValida) digitalWrite(habilitarBateria, 1);
}

void calibrarVoltajeCarga() {
  calibracionCarga = 0.0000047952 * pow(corrienteBateria, 2) - 0.0542058 * corrienteBateria + 18.3546;
  voltajeBateria += calibracionCarga;
}

void calcularTemperatura() {
  float V = temperaturaBateria;
  temperaturaBateria = 0.00000000151869 * pow(V, 3) - 0.0000110761 * pow(V, 2) + 0.04912155 * V - 30.7246364;
}