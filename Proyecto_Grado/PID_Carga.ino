float Ts_2 = 0.1; // 100 ms
float Kp_2 = 0.4, Ti_2 = 0.1, Td_2 = 0.001;

float q0_2, q1_2, q2_2;

float m_n_2 = 0.0;
float m_n1_2 = 0.0;

float error_2 = 0.0;
float e_1_2 = 0.0;
float e_2_2 = 0.0;

void calcularPID_Carga(bool modo){

  q0_2 = Kp_2 * (1 + Ts_2/(2*Ti_2) + Td_2/Ts_2);
  q1_2 = -Kp_2 * (1 - Ts_2/(2*Ti_2) + (2*Td_2)/Ts_2);
  q2_2 = Kp_2 * (Td_2/Ts_2);

  e_2_2 = e_1_2;
  e_1_2 = error_2;
  
  if (modo == false) error_2 = SetPoint - corrienteBateria; //CC
  else error_2 = 4200 - voltajeBateria;                     //CV

  m_n_2 = m_n1_2 + q0_2*error_2 + q1_2*e_1_2 + q2_2*e_2_2;
  m_n_2 = constrain(m_n_2, 0.0, 4095.0);

  m_n1_2 = m_n_2;
  
  ledcWrite(controlCarga, m_n_2);
}