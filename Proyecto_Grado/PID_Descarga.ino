float Ts = 0.1; // 100 ms
float Kp = 1, Ti = 0.25;

float q0, q1;

float m_n = 0.0;
float m_n1 = 0.0;

float error = 0.0;
float e_1 = 0.0;

void calcularPID_Descarga(){

  q0 = Kp * (1 + Ts/(2*Ti));
  q1 = -Kp * (1 - Ts/(2*Ti));

  e_1 = error;
  error = SetPoint + corrienteBateria;

  m_n = m_n1 + q0*error + q1*e_1;
  m_n = constrain(m_n, 0.0, 4095.0);

  m_n1 = m_n;

  ledcWrite(controlDescarga, m_n);
}