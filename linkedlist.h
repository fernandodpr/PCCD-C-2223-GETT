#ifndef LINKEDLIST_H
#define LINKEDLIST_H
    struct Nodo {
        int valor;
        struct Nodo* siguiente;
    };

    //FUNCIONES

    void borrarLista(struct Nodo** cabeza);
    void agregarNodo(struct Nodo** cabeza, int valor);
#endif