float Ts_2 = 0.1f;
float Kp_2 = 0.4f, Ti_2 = 0.1f, Td_2 = 0.001f;
float q0_2 = 0, q1_2 = 0, q2_2 = 0;
float m_n_2 = 0, m_n1_2 = 0;
float error_2 = 0, e_1_2 = 0, e_2_2 = 0;

void resetPID_Carga() {
  m_n_2 = m_n1_2 = error_2 = e_1_2 = e_2_2 = 0;
  ledcWrite(controlCarga, 0);
}

void calcularPID_Carga(bool modoCV) {
  q0_2 = Kp_2 * (1.0f + Ts_2 / (2.0f * Ti_2) + Td_2 / Ts_2);
  q1_2 = -Kp_2 * (1.0f - Ts_2 / (2.0f * Ti_2) + 2.0f * Td_2 / Ts_2);
  q2_2 = Kp_2 * Td_2 / Ts_2;
  float nuevoError = modoCV ? (4200.0f - voltajeBateria) : (SetPoint - corrienteBateria);
  e_2_2 = e_1_2;
  e_1_2 = error_2;
  error_2 = nuevoError;
  m_n_2 = constrain(m_n1_2 + q0_2 * error_2 + q1_2 * e_1_2 + q2_2 * e_2_2, 0.0f, 4095.0f);
  m_n1_2 = m_n_2;
  ledcWrite(controlCarga, (uint32_t)m_n_2);
}
