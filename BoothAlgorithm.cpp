#include <iostream>
#include <cstdint> // Para trabajar diferentes tamaños de bits y no el del compilador (Usamos estos porque habria irregularidades en los numeros)

int32_t booth_multiply(int16_t numero1, int16_t numero2) {
    int16_t A = 0;          // Auxiliar (inicializado a 0)
    int16_t Q = numero2; 
    int16_t M = numero1; 
    int Q_prev = 0;        // Bit anterior de Q (para las transiciones)
    const int bits = 16;   // Número de bits en un int16_t, se puede variar pero tendriamos que cambiar los tamaños de los demas

    // Bucle principal (itera una vez por cada bit, en este caso 16)
    for (int i = 0; i < bits; ++i) {
        int Q0 = Q & 1; // Bit menos significativo de Q
        int accion = (Q0 << 1) | Q_prev; // Combinar los bits para evaluar suma o resta
        switch (accion) {
            case 0b01:  // Si "01": Sumar M al acumulador (A += M)
                A += M;
                break;
            case 0b10:  // Si "10": Restar M del acumulador (A -= M)
                A -= M;
                break;
            // "00" y "11" no tiene nada como accion
        }

        //Guardar el LSB de Q antes de desplazar
        Q_prev = Q & 1;

        // Extraer el LSB de A
        int16_t bit_A = A & 1;

        // Desplazar Q a la derecha e insertar el bit de A en la posición más alta
        Q = static_cast<int16_t>( (static_cast<uint16_t>(Q) >> 1) |(static_cast<uint16_t>(bit_A)) << 15);

        // Desplazar A a la derecha (desplazamiento aritmético, guarda todavia el signo)
        A >>= 1;
    }

    //Combinamos A y Q para obtener el resultado de 32 bits, A se desplaza 16 bits a la izquierda y Q en lo restante para no tener problemas con el signo
    return (static_cast<int32_t>(A) << 16) | static_cast<uint16_t>(Q);
}

/** El algoritmo funciona para diferentes tamaños de bits, se tendria que alterar los parametros de la funcion; los desplazamientos de bits en la linea 32 y el casteo de resultado en la linea 39
**/

int main() {
    int16_t a = -223;
    int16_t b = 69; 
    int32_t resultado = booth_multiply(a, b);

    std::cout << a << " * " << b << " = " << resultado << std::endl;
    return 0;
}
