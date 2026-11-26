void enviar_Datos() {
  String p = "S=" + estadoActualSistema + "|M=" + String((int)modoOperacion);
  p += "|V=" + String(voltajeBateria, 1) + "|I=" + String(corrienteBateria, 2);
  p += "|T=" + String(temperaturaBateria, 2) + "|CC=" + String(capacidadCarga_mAh, 3);
  p += "|CD=" + String(capacidadDescarga_mAh, 3) + "|EC=" + String(energiaCarga_mWh, 3);
  p += "|ED=" + String(energiaDescarga_mWh, 3) + "|TC=" + String(tiempoCarga_s, 1);
  p += "|TD=" + String(tiempoDescarga_s, 1) + "|R=" + String(RInterna, 6);
  p += "|RV=" + String(RInternaValida ? 1 : 0) + "|BAT=" + String(bateriaValida ? 1 : 0);
  p += "|AL=" + motivoPausa;
  emitirEvento("TELEMETRIA", p);
}
