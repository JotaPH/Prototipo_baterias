void medirParamtros() {
  float voltajeADC = (analogRead(detectarBateria) * 3300.0f) / 4095.0f;
  // Se conserva el umbral de detección existente; debe corresponder al divisor real del hardware.
  if (!bateriaValida && voltajeADC >= 10.0f) {
    bateriaValida = true;
    digitalWrite(ledIndicador, HIGH);
  } else if (bateriaValida && voltajeADC <= 1.0f) {
    bateriaValida = false;
    digitalWrite(ledIndicador, LOW);
  }

  corrienteBateria = ina3221.getCurrentAmps(0) * 1000.0f;
  voltajeBateria = ina3221.getBusVoltage(1) * 1000.0f;
  calibrarVoltajeCarga();
  temperaturaBateria = ina3221.getBusVoltage(2) * 1000.0f + 6.0f;
  calcularTemperatura();
}

void actualizarReleBateria() {
  // La validez autoriza la conexión, pero no la solicita por sí sola.
  // Solo se conecta durante una operación activa; en reposo/pausa queda apagado.
  bool operacionActiva =
    estado == PRECARGA || estado == CARGA_CC || estado == CARGA_CV ||
    estado == DESCARGA ||
    (estado >= RINT_ESTABILIZAR_I1 && estado <= RINT_MUESTREAR_I2);

  digitalWrite(habilitarBateria,
               (bateriaValida && operacionActiva) ? HIGH : LOW);

  // El indicador sí representa únicamente la presencia de batería válida.
  digitalWrite(ledIndicador, bateriaValida ? HIGH : LOW);
}

void calibrarVoltajeCarga() {
  calibracionCarga = 0.0000047952f * pow(corrienteBateria, 2) - 0.0542058f * corrienteBateria + 18.3546f;
  voltajeBateria += calibracionCarga;
}

void calcularTemperatura() {
  float V = temperaturaBateria;
  temperaturaBateria = 0.00000000151869f * pow(V, 3) - 0.0000110761f * pow(V, 2) + 0.04912155f * V - 30.7246364f;
}
