clear

#Matar procesos 
killall receptor.o
killall multiproceso.o
killall multinodo.o

#Borrar ejecutables
rm receptor.o
rm multiproceso.o
rm multinodo.o
clear


clear
#Compilación del proyecto
gcc -Wall multiproceso.c network.c linkedlist.c -o multiproceso.o
#gcc -Wall multinodo.c network.c linkedlist.c -o multinodo.o
#gcc -Wall receptor.c -o receptor.o
#git-stats -a