void apagarEtapasPotencia() {
  ledcWrite(controlCarga, 0);
  ledcWrite(controlDescarga, 0);
  digitalWrite(habilitarCarga, LOW);
  digitalWrite(habilitarDescarga, HIGH); // HIGH inhibe la etapa de descarga.
  resetPID_Carga();
  resetPID_Descarga();
}

void actualizarAcumuladores(float dt) {
  if (dt <= 0 || dt > 1.0f) return;
  if (estado == PRECARGA || estado == CARGA_CC || estado == CARGA_CV) {
    float i = fabs(corrienteBateria);
    capacidadCarga_mAh += i * dt / 3600.0f;
    energiaCarga_mWh += fabs(voltajeBateria) * i * dt / 3600000.0f;
    tiempoCarga_s += dt;
  } else if (estado == DESCARGA || estado == RINT_ESTABILIZAR_I1 ||
             estado == RINT_MUESTREAR_I1 || estado == RINT_ESTABILIZAR_I2 ||
             estado == RINT_MUESTREAR_I2) {
    float i = fabs(corrienteBateria);
    capacidadDescarga_mAh += i * dt / 3600.0f;
    energiaDescarga_mWh += fabs(voltajeBateria) * i * dt / 3600000.0f;
    tiempoDescarga_s += dt;
  }
}

void activarCarga() {
  ledcWrite(controlDescarga, 0);
  digitalWrite(habilitarDescarga, HIGH);
  digitalWrite(habilitarCarga, LOW);
  if (!bateriaValida) return;
  digitalWrite(habilitarBateria, HIGH);
  digitalWrite(habilitarCarga, HIGH);
  calcularPID_Carga(estado == CARGA_CV);
}

void activarDescarga() {
  ledcWrite(controlCarga, 0);
  digitalWrite(habilitarCarga, LOW);
  if (!bateriaValida) {
    ledcWrite(controlDescarga, 0);
    digitalWrite(habilitarDescarga, HIGH);
    return;
  }
  digitalWrite(habilitarBateria, HIGH);
  digitalWrite(habilitarDescarga, LOW);
  calcularPID_Descarga();
}

void iniciarOperacion() {
  if (!configuracionRecibida || !modoRecibido || !bateriaValida || alarmaActiva) return;
  motivoPausa = "";
  if (estado == PAUSADO && estadoReanudacion != IDLE && estadoReanudacion != PAUSADO) {
    estado = estadoReanudacion;
  } else if (modoOperacion == MODO_CARGA) {
    if (voltajeBateria < 2000.0f && !permitirPrecargaBajo2V) {
      pausarSistema("PRECARGA_BLOQUEADA_BAJO_2V", false);
      return;
    }
    estado = (voltajeBateria < voltajePrecarga) ? PRECARGA : CARGA_CC;
  } else if (modoOperacion == MODO_DESCARGA) {
    estado = DESCARGA;
  }
  resetPID_Carga();
  resetPID_Descarga();
  tiempoAcumuladoresAnterior = millis();
  actualizarTextoEstado();
  emitirEvento("SISTEMA_INICIADO", estadoActualSistema);
}

void pausarSistema(const String &motivo, bool porAlarma) {
  if (estado == PAUSADO || estado == FALLO) return;
  if (estado != IDLE && estado != FINALIZADO) estadoReanudacion = estado;
  bool rintInterrumpida = (estado >= RINT_ESTABILIZAR_I1 && estado <= RINT_MUESTREAR_I2) || rintSolicitada;
  if (rintInterrumpida) {
    emitirEvento("RINT_INTERRUMPIDA", "TEST=" + String(idPruebaR) + "|MOTIVO=" + motivo);
    rintSolicitada = false;
    rintEnCurso = false;
    RInternaValida = false;
    SetPoint = SetPointUsuario;
    if (estadoReanudacion >= RINT_ESTABILIZAR_I1 && estadoReanudacion <= RINT_MUESTREAR_I2)
      estadoReanudacion = DESCARGA;
  }
  apagarEtapasPotencia();
  estado = PAUSADO;
  motivoPausa = motivo;
  alarmaActiva = porAlarma;
  actualizarTextoEstado();
  emitirEvento("SISTEMA_PAUSADO", "MOTIVO=" + motivo);
}

void resetSistema() {
  apagarEtapasPotencia();
  estado = IDLE;
  estadoReanudacion = IDLE;
  modoOperacion = MODO_NINGUNO;
  cargarBateria = descargarBateria = medirR = false;
  SetPoint = SetPointUsuario = 500.0f;
  corrienteMinima = 100.0f;
  voltajeMinimo = 2900.0f;
  corrientePrecarga = 100.0f;
  voltajePrecarga = 2900.0f;
  permitirPrecargaBajo2V = false;
  capacidadConocida = false;
  capacidadConocida_mAh = 0.0f;
  configuracionRecibida = false;
  modoRecibido = false;
  capacidadCarga_mAh = capacidadDescarga_mAh = 0.0f;
  energiaCarga_mWh = energiaDescarga_mWh = 0.0f;
  tiempoCarga_s = tiempoDescarga_s = 0.0f;
  RInterna = 0.0f;
  RInternaValida = false;
  I1_R = 200.0f; I2_R = 1000.0f;
  t1_R_ms = 10000; t2_R_ms = 2000;
  idPruebaR = 0;
  rintSolicitada = rintEnCurso = false;
  motivoPausa = "";
  alarmaActiva = false;
  muestrasSobreVoltaje = muestrasSobreCorriente = muestrasSobreTemperatura = 0;
  resetPID_Carga();
  resetPID_Descarga();
  tiempoCargaAnterior = tiempoDescargaAnterior = millis();
  tiempoAcumuladoresAnterior = millis();
  actualizarTextoEstado();
  emitirEvento("RESET_COMPLETADO", "CONFIGURACION_PREDETERMINADA");
}

void actualizarTextoEstado() {
  switch (estado) {
    case IDLE: estadoActualSistema = "IDLE"; estadoActualTexto = "En reposo"; break;
    case PAUSADO: estadoActualSistema = "PAUSADO"; estadoActualTexto = "Pausado: " + motivoPausa; break;
    case PRECARGA: estadoActualSistema = "PRECARGA"; estadoActualTexto = "Precarga"; break;
    case CARGA_CC: estadoActualSistema = "CARGA_CC"; estadoActualTexto = "Carga CC"; break;
    case CARGA_CV: estadoActualSistema = "CARGA_CV"; estadoActualTexto = "Carga CV"; break;
    case DESCARGA: estadoActualSistema = "DESCARGA"; estadoActualTexto = "Descargando"; break;
    case RINT_ESTABILIZAR_I1: estadoActualSistema = "RINT_EST_I1"; estadoActualTexto = "Estabilizando I1"; break;
    case RINT_MUESTREAR_I1: estadoActualSistema = "RINT_MUE_I1"; estadoActualTexto = "Midiendo I1"; break;
    case RINT_ESTABILIZAR_I2: estadoActualSistema = "RINT_EST_I2"; estadoActualTexto = "Estabilizando I2"; break;
    case RINT_MUESTREAR_I2: estadoActualSistema = "RINT_MUE_I2"; estadoActualTexto = "Midiendo I2"; break;
    case FINALIZADO: estadoActualSistema = "FINALIZADO"; estadoActualTexto = "Operación finalizada"; break;
    case FALLO: estadoActualSistema = "FALLO"; estadoActualTexto = "Fallo"; break;
  }
}

void ejecutarMaquinaEstados() {
  EstadoSistema anterior = estado;
  if (estado == IDLE || estado == PAUSADO || estado == FINALIZADO || estado == FALLO) {
    apagarEtapasPotencia();
  } else if (!bateriaValida) {
    pausarSistema("BATERIA_NO_DETECTADA", true);
  } else {
    switch (estado) {
      case PRECARGA:
        SetPoint = corrientePrecarga;
        activarCarga();
        if (voltajeBateria >= voltajePrecarga) {
          estado = CARGA_CC;
          resetPID_Carga();
        }
        break;
      case CARGA_CC:
        SetPoint = SetPointUsuario;
        activarCarga();
        if (voltajeBateria >= 4200.0f) {
          estado = CARGA_CV;
          resetPID_Carga();
        }
        break;
      case CARGA_CV:
        activarCarga();
        if (corrienteBateria <= corrienteMinima) {
          apagarEtapasPotencia();
          estado = FINALIZADO;
          emitirEvento("CARGA_FINALIZADA", "");
        }
        break;
      case DESCARGA:
        activarDescarga();
        if (voltajeBateria <= voltajeMinimo) {
          apagarEtapasPotencia();
          estado = FINALIZADO;
          rintSolicitada = false;
          emitirEvento("DESCARGA_FINALIZADA", "");
        } else if (rintSolicitada && capacidadConocida &&
                   capacidadDescarga_mAh >= capacidadConocida_mAh * 0.5f) {
          estado = RINT_ESTABILIZAR_I1;
          rintEnCurso = true;
          muestrasEstablesR = 0;
          sumaI_R = sumaV_R = 0;
          muestrasR = 0;
          SetPoint = I1_R;
          resetPID_Descarga();
          emitirEvento("RINT_INICIO", "TEST=" + String(idPruebaR));
        }
        break;
      case RINT_ESTABILIZAR_I1:
      case RINT_ESTABILIZAR_I2:
        SetPoint = (estado == RINT_ESTABILIZAR_I1) ? I1_R : I2_R;
        activarDescarga();
        toleranciaR_mA = max(5.0f, 0.02f * fabs(SetPoint));
        if (fabs(fabs(corrienteBateria) - SetPoint) <= toleranciaR_mA) {
          if (muestrasEstablesR < 10) muestrasEstablesR++;
        } else muestrasEstablesR = 0;
        if (muestrasEstablesR >= 10) {
          muestrasEstablesR = 0;
          sumaI_R = sumaV_R = 0;
          muestrasR = 0;
          inicioMuestreoR = millis();
          estado = (estado == RINT_ESTABILIZAR_I1) ? RINT_MUESTREAR_I1 : RINT_MUESTREAR_I2;
        }
        break;
      case RINT_MUESTREAR_I1:
      case RINT_MUESTREAR_I2: {
        bool primera = estado == RINT_MUESTREAR_I1;
        SetPoint = primera ? I1_R : I2_R;
        activarDescarga();
        sumaI_R += fabs(corrienteBateria);
        sumaV_R += voltajeBateria;
        muestrasR++;
        uint32_t duracion = primera ? t1_R_ms : t2_R_ms;
        if (millis() - inicioMuestreoR >= duracion && muestrasR > 0) {
          float iProm = sumaI_R / muestrasR;
          float vProm = sumaV_R / muestrasR;
          if (primera) {
            promedioI1 = iProm; promedioV1 = vProm;
            estado = RINT_ESTABILIZAR_I2;
            resetPID_Descarga();
          } else {
            promedioI2 = iProm; promedioV2 = vProm;
            float deltaI = promedioI2 - promedioI1;
            if (deltaI > 0.1f) {
              RInterna = fabs((promedioV1 - promedioV2) / deltaI);
              RInternaValida = isfinite(RInterna) && RInterna > 0;
            SetPoint = SetPointUsuario;
            } else RInternaValida = false;
            rintEnCurso = false;
            rintSolicitada = false;
            estado = DESCARGA;
            resetPID_Descarga();
            emitirEvento("RINT_FINALIZADA", "TEST=" + String(idPruebaR) +
              "|R_OHM=" + String(RInterna, 6) + "|VALIDA=" + String(RInternaValida ? 1 : 0));
          }
        }
        break;
      }
      default: break;
    }
  }
  if (estado != anterior) {
    actualizarTextoEstado();
    emitirEvento("CAMBIO_ESTADO", estadoActualSistema);
  }
}
