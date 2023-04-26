from datetime import datetime

import collections

# Define the linked list
LinkedList = collections.OrderedDict()
# Abrir el archivo y leer las líneas
with open('registro.txt') as file:
    lines = file.readlines()

lines = [line.strip() for line in lines if line.strip()]

# Crear un nodo para cada línea y agregarlo a la linked list
for line in lines:
    # Extraer el timestamp y el valor entra/sale
    timestamp_str, value = line.split('] ')[0][1:], line.split('] ')[1].strip()

    # Convertir el timestamp a datetime
    timestamp = datetime.strptime(timestamp_str, '%H:%M:%S.%f')

    # Crear el nodo y agregarlo a la linked list
    linkedlist.add(timestamp.timestamp(), value)

# Ordenar la linked list por orden temporal
linkedlist.sort()

# Recorrer la linked list y eliminar los nodos cuyo valor es igual al valor del nodo anterior
prev_node = None
node = linkedlist.head
while node:
    if prev_node and node.value == prev_node.value:
        linkedlist.remove(node.id)
    else:
        prev_node = node
    node = node.next

# Imprimir la linked list resultante
linkedlist.print()
