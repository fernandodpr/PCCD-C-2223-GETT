import csv
from datetime import datetime

# Abrir el archivo CSV y leer los datos
with open('100nodos100procesos.csv', newline='') as csvfile:
    procesos_csv = csv.DictReader(csvfile)
    
    # Inicializar una lista para almacenar las diferencias de tiempo
    diferencias_tiempo = []
    
    # Procesar la primera fila de datos
    primer_proceso = next(procesos_csv)
    hora_creacion = datetime.strptime(primer_proceso['Hora de creación del proceso'], '%H:%M:%S.%f')
    hora_entrada = datetime.strptime(primer_proceso['Hora de entrada a la SC'], '%H:%M:%S.%f')
    diferencia = hora_entrada - hora_creacion
    print(f"El retraso de la línea 1 es de {diferencia.total_seconds():.3f} segundos")
    diferencias_tiempo.append(diferencia.total_seconds())
    hora_salida = datetime.strptime(primer_proceso['Hora de salida de la SC'], '%H:%M:%S.%f')
    
    # Procesar las filas de datos restantes
    for proceso in procesos_csv:
        hora_anterior = hora_salida
        hora_entrada = datetime.strptime(proceso['Hora de entrada a la SC'], '%H:%M:%S.%f')
        diferencia = hora_entrada - hora_anterior
        diferencias_tiempo.append(diferencia.total_seconds())
        hora_salida = datetime.strptime(proceso['Hora de salida de la SC'], '%H:%M:%S.%f')
    
    # Calcular la media de las diferencias de tiempo
    media = sum(diferencias_tiempo) / len(diferencias_tiempo)
    
    # Imprimir la media
    print(f"La media de las diferencias de tiempo es de {media:.3f} segundos")
