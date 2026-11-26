extern float corrienteMinima;
extern float voltajeMinimo;

void enviar_Datos(){
  Serial.println("================================");
  Serial.print("Sistema = ");
  Serial.println(estadoActualSistema);
  Serial.printf("Setpoint = %07.2f\n", SetPoint);
  Serial.printf("Corriente = %07.2f\n", corrienteBateria);
  Serial.printf("Voltaje = %07.2f\n", voltajeBateria);
  Serial.printf("Temperatura = %05.2f %07.2f\n", temperaturaBateria, 0.0241 * temperaturaBateria - 13.517);

  Serial.print("Estado = ");
  Serial.println(estadoActualTexto);

  Serial.printf("Corriente minima = %06.2f\n", corrienteMinima);
  Serial.printf("Voltaje minimo = %07.2f\n", voltajeMinimo);

  Serial.print("Bateria válida = ");
  Serial.println(bateriaValida);

  Serial.print("Capacidad = ");
  Serial.println(String(capacidad_mAh) + " mAh");

  Serial.printf("Tiempo = %02lu:%02lu\n", (unsigned long)(tiempoTranscurrido / 60.0), (unsigned long)tiempoTranscurrido % 60);

  Serial.print("RInterna = ");
  Serial.println(String(RInterna, 6));

  Serial.println("================================");
}