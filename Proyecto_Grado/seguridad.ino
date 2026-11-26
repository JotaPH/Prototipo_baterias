void revisarSeguridad() {
  // Se siguen leyendo las condiciones durante PAUSA para permitir la recuperación.
  muestrasSobreVoltaje = (voltajeBateria > 4300.0f) ? muestrasSobreVoltaje + 1 : 0;
  muestrasSobreCorriente = (fabs(corrienteBateria) > 2100.0f) ? muestrasSobreCorriente + 1 : 0;
  muestrasSobreTemperatura = (temperaturaBateria > 60.0f) ? muestrasSobreTemperatura + 1 : 0;

  bool condicionesSeguras = bateriaValida &&
    muestrasSobreVoltaje == 0 && muestrasSobreCorriente == 0 &&
    muestrasSobreTemperatura == 0;

  if (estado == PAUSADO && alarmaActiva && condicionesSeguras) {
    alarmaActiva = false;
    emitirEvento("ALARMA_RESTABLECIDA", "REQUIERE_INICIO_MANUAL");
    return;
  }
  if (estado == IDLE || estado == PAUSADO || estado == FINALIZADO || estado == FALLO) return;

  if (!bateriaValida) {
    pausarSistema("BATERIA_NO_DETECTADA", true);
  } else if (muestrasSobreVoltaje >= 6) {
    pausarSistema("SOBREVOLTAJE", true);
  } else if (muestrasSobreCorriente >= 6) {
    pausarSistema("SOBRECORRIENTE", true);
  } else if (muestrasSobreTemperatura >= 6) {
    pausarSistema("SOBRETEMPERATURA", true);
  }
}