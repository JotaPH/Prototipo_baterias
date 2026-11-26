// La secuencia de R interna se ejecuta desde la máquina de estados.
// La interfaz solicita RINT(I1,t1,I2,t2). Solo se admite con capacidad conocida
// y modo DESCARGA. La prueba comienza al alcanzar el 50% de capacidad descargada.
// La estabilización exige 10 muestras consecutivas dentro de max(5mA, 2% SP).
// Una pausa durante la prueba invalida el resultado parcial y emite RINT_INTERRUMPIDA.
