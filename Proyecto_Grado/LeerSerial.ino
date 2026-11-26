// Protocolo: @ID|COMANDO|PARAMETROS|CRC16\n
// CRC-16/CCITT-FALSE sobre "ID|COMANDO|PARAMETROS" (sin @ ni |CRC).
// CONFIG: SP_mA,Imin_mA,Vmin_mV,Ipre_mA,Vpre_mV,allowBelow2,capKnown,capacidad_mAh
// MODO: CARGA o DESCARGA. RINT: I1_mA,t1_s,I2_mA,t2_s.

static char rxBuffer[220];
static uint16_t rxLength = 0;
static bool rxOverflow = false;
static uint32_t ultimoIdRX = 0;
static uint32_t idEventoTX = 50000;

uint16_t crc16_ccitt(const char *data) {
  uint16_t crc = 0xFFFF;
  while (*data) {
    crc ^= (uint16_t)(uint8_t)(*data++) << 8;
    for (uint8_t i = 0; i < 8; i++)
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
  }
  return crc;
}

void enviarFrame(uint32_t id, const char *tipo, const String &payload) {
  String base = String(id) + "|" + tipo + "|" + payload;
  uint16_t crc = crc16_ccitt(base.c_str());
  char crcText[5];
  snprintf(crcText, sizeof(crcText), "%04X", crc);
  Serial.print("@");
  Serial.print(base);
  Serial.print("|");
  Serial.print(crcText);
  Serial.print("\n");
}

void responder(uint32_t id, const char *tipo, const String &payload) {
  enviarFrame(id, tipo, payload);
}

void emitirEvento(const char *evento, const String &payload) {
  enviarFrame(idEventoTX++, "EVENTO", String(evento) + (payload.length() ? "|" + payload : ""));
}

String campoCSV(const String &s, uint8_t indice) {
  int inicio = 0;
  for (uint8_t i = 0; i < indice; i++) {
    int p = s.indexOf(',', inicio);
    if (p < 0) return "";
    inicio = p + 1;
  }
  int fin = s.indexOf(',', inicio);
  if (fin < 0) fin = s.length();
  return s.substring(inicio, fin);
}

bool numeroValido(const String &s, float &v) {
  if (!s.length()) return false;
  char *endp = nullptr;
  v = strtof(s.c_str(), &endp);
  return endp != s.c_str() && *endp == '\0' && isfinite(v);
}

void responderEstado(uint32_t id) {
  String p = estadoActualSistema + "," + estadoActualTexto + "," + motivoPausa + ",";
  p += String((int)modoOperacion) + "," + String(voltajeBateria, 1) + ",";
  p += String(corrienteBateria, 2) + "," + String(capacidadCarga_mAh, 3) + ",";
  p += String(capacidadDescarga_mAh, 3) + "," + String(energiaCarga_mWh, 3) + ",";
  p += String(energiaDescarga_mWh, 3) + "," + String(tiempoCarga_s, 1) + ",";
  p += String(tiempoDescarga_s, 1) + "," + String(RInterna, 6) + ",";
  p += String(RInternaValida ? 1 : 0) + "," + String(bateriaValida ? 1 : 0);
  responder(id, "ESTADO", p);
}

void procesarComando(const String &base, uint32_t id, const String &cmd, const String &payload) {
  if (id == ultimoIdRX) {
    responder(id, "ACK_DUPLICADO", "YA_PROCESADO");
    return;
  }
  if (id < ultimoIdRX) {
    responder(id, "NACK", "ID_ANTIGUO");
    return;
  }

  bool aceptado = false;
  String error = "COMANDO_O_PARAMETROS_INVALIDOS";

  if (cmd == "CONFIG") {
    float v[8];
    bool ok = true;
    for (uint8_t i = 0; i < 8; i++) ok &= numeroValido(campoCSV(payload, i), v[i]);
    if (ok && v[0] > 0 && v[0] <= 2000 && v[1] >= 0 && v[1] <= 2100 &&
        v[2] >= 2000 && v[2] <= 4200 && v[3] > 0 && v[3] <= 500 &&
        v[4] >= 2000 && v[4] <= 4200 && (v[5] == 0 || v[5] == 1) &&
        (v[6] == 0 || v[6] == 1) && v[7] >= 0 && v[7] <= 30000 &&
        ((v[6] == 0 && v[7] == 0) || (v[6] == 1 && v[7] > 0)) &&
        (estado == IDLE || estado == PAUSADO || estado == FINALIZADO)) {
      SetPoint = SetPointUsuario = v[0]; corrienteMinima = v[1]; voltajeMinimo = v[2];
      corrientePrecarga = v[3]; voltajePrecarga = v[4];
      permitirPrecargaBajo2V = v[5] == 1;
      capacidadConocida = v[6] == 1;
      capacidadConocida_mAh = v[7];
      configuracionRecibida = true;
      aceptado = true;
    }
  } else if (cmd == "MODO") {
    if (payload == "CARGA" || payload == "DESCARGA") {
      if (estado == IDLE || estado == PAUSADO || estado == FINALIZADO) {
        modoOperacion = payload == "CARGA" ? MODO_CARGA : MODO_DESCARGA;
        cargarBateria = modoOperacion == MODO_CARGA;
        descargarBateria = modoOperacion == MODO_DESCARGA;
        modoRecibido = true;
        aceptado = true;
      } else error = "PAUSAR_ANTES_DE_CAMBIAR_MODO";
    }
  } else if (cmd == "SETPOINT") {
    float v;
    if (numeroValido(payload, v) && v > 0 && v <= 2000 &&
        (estado == IDLE || estado == PAUSADO || estado == FINALIZADO)) {
      SetPoint = SetPointUsuario = v;
      aceptado = true;
    } else error = "SETPOINT_INVALIDO_O_SISTEMA_ACTIVO";
  } else if (cmd == "RINT") {
    float v[4];
    bool ok = true;
    for (uint8_t i = 0; i < 4; i++) ok &= numeroValido(campoCSV(payload, i), v[i]);
    if (ok && capacidadConocida && modoOperacion == MODO_DESCARGA &&
        v[0] > 0 && v[0] < v[2] && v[2] <= 2000 &&
        v[1] >= 1 && v[1] <= 120 && v[3] >= 1 && v[3] <= 120 &&
        !rintEnCurso && !rintSolicitada &&
        (estado == DESCARGA || estado == PAUSADO || estado == IDLE)) {
      I1_R = v[0]; t1_R_ms = (uint32_t)(v[1] * 1000.0f);
      I2_R = v[2]; t2_R_ms = (uint32_t)(v[3] * 1000.0f);
      rintSolicitada = true;
      idPruebaR++;
      RInternaValida = false;
      aceptado = true;
      emitirEvento("RINT_SOLICITADA", "TEST=" + String(idPruebaR));
    } else error = !capacidadConocida ? "CAPACIDAD_DESCONOCIDA" : "RINT_NO_PERMITIDA_EN_ESTADO_ACTUAL";
  } else if (cmd == "INICIO") {
    if (!configuracionRecibida || !modoRecibido) error = "FALTA_CONFIGURACION_O_MODO";
    else if (!bateriaValida) error = "BATERIA_NO_DETECTADA";
    else if (alarmaActiva) error = "ALARMA_ACTIVA";
    else if (estado == PAUSADO || estado == IDLE || estado == FINALIZADO) {
      iniciarOperacion();
      aceptado = estado != PAUSADO;
      if (estado == PAUSADO) error = motivoPausa;
    } else error = "SISTEMA_YA_ACTIVO";
  } else if (cmd == "PAUSA") {
    pausarSistema("PAUSA_MANUAL", false);
    aceptado = true;
  } else if (cmd == "RESET") {
    resetSistema();
    aceptado = true;
  } else if (cmd == "ESTADO") {
    ultimoIdRX = id;
    responderEstado(id);
    return;
  }

  if (aceptado) {
    ultimoIdRX = (cmd == "RESET") ? 0 : id;
    responder(id, "ACK", cmd + "_ACEPTADO");
  } else {
    responder(id, "NACK", error);
  }
}

void procesarLinea(char *linea) {
  if (linea[0] != '@') return;
  char *crcSep = strrchr(linea, '|');
  if (!crcSep || strlen(crcSep + 1) != 4) return;
  char recibido[5];
  memcpy(recibido, crcSep + 1, 4); recibido[4] = '\0';
  char *endp = nullptr;
  uint16_t crcRecibido = (uint16_t)strtoul(recibido, &endp, 16);
  if (!endp || *endp != '\0') return;
  *crcSep = '\0';
  const char *base = linea + 1;
  if (crc16_ccitt(base) != crcRecibido) {
    responder(0, "NACK", "CRC_INVALIDO");
    return;
  }

  char *sep1 = strchr((char *)base, '|');
  if (!sep1) return;
  *sep1 = '\0';
  char *sep2 = strchr(sep1 + 1, '|');
  if (!sep2) return;
  *sep2 = '\0';
  uint32_t id = strtoul(base, nullptr, 10);
  String cmd = String(sep1 + 1);
  String payload = String(sep2 + 1);
  procesarComando(String(base), id, cmd, payload);
}

void leerSerial() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      if (!rxOverflow && rxLength > 0) {
        rxBuffer[rxLength] = '\0';
        procesarLinea(rxBuffer);
      } else if (rxOverflow) {
        responder(0, "NACK", "TRAMA_DEMASIADO_LARGA");
      }
      rxLength = 0;
      rxOverflow = false;
    } else if (rxLength < sizeof(rxBuffer) - 1) {
      if (rxLength == 0 && c != '@') continue;
      rxBuffer[rxLength++] = c;
    } else {
      rxOverflow = true;
    }
  }
}
