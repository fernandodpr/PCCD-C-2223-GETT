import csv
from datetime import datetime, timedelta

archivo_csv = 'historialporordendeejecucion_1.csv'

# Diccionarios para almacenar los tiempos de retardo por prioridad y la cantidad de procesos por prioridad
tiempos_por_prioridad = {1: [], 2: [], 3: [], 4: [], 5: []}
conteo_por_prioridad = {1: 0, 2: 0, 3: 0, 4: 0, 5: 0}

# Variables para almacenar la media global y el total de procesos
media_global = 0
total_procesos = 0

# Lectura del archivo CSV
with open(archivo_csv) as archivo:
    lector_csv = csv.DictReader(archivo)

    for proceso in lector_csv:
        prioridad = int(proceso['Prioridad'])
        hora_creacion = datetime.strptime(proceso['Hora de creación del proceso'], '%H:%M:%S.%f')
        hora_entrada = datetime.strptime(proceso['Hora de entrada a la SC'], '%H:%M:%S.%f')
        retardo = (hora_entrada - hora_creacion).total_seconds()

        # Agregar el retardo a la lista correspondiente a la prioridad del proceso
        tiempos_por_prioridad[prioridad].append(retardo)
        conteo_por_prioridad[prioridad] += 1

        # Sumar el retardo al total y al conteo de procesos
        media_global += retardo
        total_procesos += 1

# Calcular la media global de los retardos
if total_procesos > 0:
    media_global /= total_procesos

# Imprimir los resultados por prioridad y la media global
for prioridad in range(1, 6):
    tiempos_en_segundos = tiempos_por_prioridad[prioridad]
    media_prioridad = sum(tiempos_en_segundos) / conteo_por_prioridad[prioridad] if conteo_por_prioridad[prioridad] > 0 else 0
    print(f'Tiempo de retardo medio para prioridad {prioridad}: {media_prioridad:.2f} segundos')

print(f'Media global de tiempo de retardo: {media_global:.2f} segundos')
