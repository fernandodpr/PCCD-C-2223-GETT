clear

#Matar procesos 
killall receptor
killall multiproceso
killall multinodo

#Borrar ejecutables
rm receptor
rm multiproceso
rm multinodo

#Compilación del proyecto
gcc -Wall multiproceso.c network.c linkedlist.c -o multiproceso
gcc -Wall multinodo.c network.c linkedlist.c -o multinodo
gcc -Wall receptor.c -o receptor