import csv

# Abrir archivo CSV
with open('archivo10x10.csv', 'r') as f:
    reader = csv.reader(f)
    # Ordenar filas por hora:minuto:segundo.milisegundos
    sorted_rows = sorted(reader, key=lambda row: row[0])

# Escribir filas ordenadas en un nuevo archivo CSV
with open('archivo_ordenado.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    for row in sorted_rows:
        writer.writerow(row)