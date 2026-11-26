# Protocolo serial del ESP32

Configuración física: UART USB a 115200 baudios, 8N1. Cada trama termina en LF (\\n).

## Formato

`@ID|COMANDO|PARAMETROS|CRC16\\n`

- `@`: inicio.
- `ID`: entero decimal estrictamente creciente durante la sesión del ESP32.
- `COMANDO`: nombre de operación.
- `PARAMETROS`: campos separados por coma; puede estar vacío.
- `CRC16`: CRC-16/CCITT-FALSE, polinomio `0x1021`, valor inicial `0xFFFF`, sin reflexión, XOR final `0x0000`. Se calcula sobre los bytes ASCII de `ID|COMANDO|PARAMETROS`, sin `@` ni el separador anterior al CRC. Se transmite en cuatro dígitos hexadecimales mayúsculos.

Los IDs de mensajes no se reinician con RESET: deben continuar creciendo para que una retransmisión antigua no vuelva a ejecutar un comando. El identificador de prueba de resistencia interna sí se reinicia a cero al hacer RESET y vuelve a uno en la primera prueba de la batería nueva.

## Comandos

### CONFIG

Ocho campos, en este orden:

`SP_mA,Imin_mA,Vmin_mV,Iprecarga_mA,Vprecarga_mV,permitirBajo2V,capacidadConocida,capacidad_mAh`

Los dos indicadores son 0 o 1. Si la capacidad es desconocida, enviar `0,0` en los dos últimos campos.

Ejemplo:

```text
@1|CONFIG|500,100,2900,100,2900,0,1,2500|6D0E
```

### MODO

`CARGA` o `DESCARGA`.

```text
@2|MODO|DESCARGA|BED8
```

### INICIO, PAUSA, RESET, ESTADO

```text
@3|INICIO||119D
@4|PAUSA||6DFD
@5|ESTADO||EAE4
@6|RESET||CAC1
```

RESET borra los resultados y devuelve la configuración a valores predeterminados. La interfaz debe enviar CONFIG y MODO de nuevo antes de INICIO.

### SETPOINT

Un valor en mA. Se acepta con el sistema en reposo, pausado o finalizado.

### RINT

Cuatro campos: `I1_mA,t1_s,I2_mA,t2_s`. Solo se acepta en modo DESCARGA y con capacidad conocida. El firmware espera hasta alcanzar el 50 % de la capacidad conocida descargada, estabiliza cada escalón durante diez muestras consecutivas a 100 ms (tolerancia `max(5 mA, 2 % de consigna)`) y luego promedia corriente y voltaje durante cada duración.

Ejemplo:

```text
@7|RINT|200,10,1000,2|7022
```

## Respuestas

- `ACK`: comando aceptado.
- `ACK_DUPLICADO`: ya se procesó el mismo ID.
- `NACK`: comando rechazado; el payload indica el motivo.
- `ESTADO`: respuesta a la consulta ESTADO.
- `EVENTO`: eventos asíncronos, como cambio de estado, alarma, pausa, reanudación, fin de carga/descarga y resultado o interrupción de R interna.

Los eventos usan IDs de transmisión desde 50000. La interfaz debe distinguirlos de los IDs de comando entrantes por la dirección/tipo de trama.

## Notas de operación

- El firmware exige CONFIG y MODO antes de INICIO.
- La batería debe estar detectada como válida.
- Una alarma pausa el sistema y conserva acumuladores; una vez que las lecturas vuelven a condiciones seguras, el ESP32 emite ALARMA_RESTABLECIDA. La interfaz debe enviar INICIO para reanudar.
- La interrupción de R interna emite RINT_INTERRUMPIDA y descarta el resultado parcial.
- La capacidad y energía de carga/descarga, así como los tiempos de carga y descarga, se acumulan por separado.
