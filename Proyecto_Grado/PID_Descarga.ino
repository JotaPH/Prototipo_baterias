float Kp = 1.0f, Ti = 0.25f;
float q0 = 0, q1 = 0;
float m_n = 0, m_n1 = 0;
float error = 0, e_1 = 0;

void resetPID_Descarga() {
  m_n = m_n1 = error = e_1 = 0;
  ledcWrite(controlDescarga, 0);
}

void calcularPID_Descarga() {
  q0 = Kp * (1.0f + Ts / (2.0f * Ti));
  q1 = -Kp * (1.0f - Ts / (2.0f * Ti));
  e_1 = error;
  error = SetPoint + corrienteBateria; // La corriente de descarga se representa negativa.
  m_n = constrain(m_n1 + q0 * error + q1 * e_1, 0.0f, 4095.0f);
  m_n1 = m_n;
  ledcWrite(controlDescarga, (uint32_t)m_n);
}
