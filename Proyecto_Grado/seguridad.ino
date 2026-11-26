void revisarSeguridad() {
  if (estado == IDLE || estado == PAUSADO || estado == FINALIZADO || estado == FALLO) return;

  muestrasSobreVoltaje = (voltajeBateria > 4300.0f) ? muestrasSobreVoltaje + 1 : 0;
  muestrasSobreCorriente = (fabs(corrienteBateria) > 2100.0f) ? muestrasSobreCorriente + 1 : 0;
  muestrasSobreTemperatura = (temperaturaBateria > 60.0f) ? muestrasSobreTemperatura + 1 : 0;

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
