import csv
import datetime

with open('historialdeejecucion.csv') as csvfile:
    reader = csv.DictReader(csvfile)
    procesos = list(reader)

# Ordenar procesos por hora de entrada a la sección crítica
procesos_ordenados = sorted(procesos, key=lambda p: datetime.datetime.strptime(p['Hora de entrada a la SC'], '%H:%M:%S.%f').strftime('%H:%M:%S.%1f'))

# Iterar sobre procesos y verificar si hay una violación de la sección crítica
violSC = False
for i in range(len(procesos_ordenados) - 1):
    proceso_actual = procesos_ordenados[i]
    proceso_siguiente = procesos_ordenados[i + 1]
    if datetime.datetime.strptime(proceso_actual['Hora de salida de la SC'], '%H:%M:%S.%f').strftime('%H:%M:%S.%1f') > datetime.datetime.strptime(proceso_siguiente['Hora de entrada a la SC'], '%H:%M:%S.%f').strftime('%H:%M:%S.%1f'):
        violSC = True
        print(f"Se ha violado la sección crítica por el proceso {proceso_actual['ID Proceso']} con ticket {proceso_actual['Ticket']}")
        break

if not violSC:
    print("No se ha violado la sección crítica")
