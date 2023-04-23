#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Uso: $0 <cantidad>"
  exit 1
fi

n=$1
nodes=()  # Almacenar las direcciones de los nodos

for (( i=1; i<=$n; i++ )); do
  nodes+=("$i")
done

procs=()  # Almacenar los PIDs de los procesos ejecutados

for (( i=0; i<$n; i++ )); do
  args=("${nodes[$i]}")
  for (( j=i+1; j<$n+$i; j++ )); do
    index=$((j % n))
    args+=("${nodes[$index]}")
  done
  ./multiproceso.o "${args[@]}" &
  procs+=($!)  # Añadir el PID del proceso a la lista
done

echo "Los procesos se están ejecutando..."

# Capturar la señal SIGINT (Ctrl+C) y detener los procesos
function stop_processes {
  for pid in "${procs[@]}"; do
    pkill -P "$pid"  # Detener los procesos secundarios (hijos)
    kill "$pid"  # Detener el proceso principal (padre)
  done
  echo "Procesos detenidos"
}

trap stop_processes INT  # Ejecutar la función stop_processes al recibir la señal SIGINT

# Esperar a que todos los procesos terminen
wait

echo "Todos los procesos han terminado."