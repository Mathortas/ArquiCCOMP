#include <iostream>
#include <cstdint>
#include <cmath>
#include <cstring>  // Se necesita para usar memcpy

// Función que descompone un float en sus tres partes: signo, exponente y parteSignificativa.
// Nota: la "parteSignificativa" es lo que comúnmente se llama "mantisa" sin contar el bit implícito.
void descomponer(float num, uint32_t &signo, uint32_t &exp, uint32_t &parteSignificativa) {
    uint32_t bits;  
    // Copia la representación binaria de 'num' (4 bytes) en la variable 'bits'
    std::memcpy(&bits, &num, sizeof(float));
    // El bit más significativo (bit 31) representa el signo.
    signo = bits >> 31;
    // Los siguientes 8 bits (bits 30 a 23) representan el exponente.
    exp = (bits >> 23) & 0xFF;
    // Los 23 bits menos significativos (bits 22 a 0) son la parteSignificativa.
    parteSignificativa = bits & 0x7FFFFF;
}

// Función que recompone un float a partir de sus componentes: signo, exponente y parteSignificativa.
float componer(uint32_t signo, uint32_t exp, uint32_t parteSignificativa) {
    // Se unen las partes en un único entero de 32 bits, ubicando cada parte en la posición correspondiente.
    uint32_t bits = (signo << 31) | (exp << 23) | parteSignificativa;
    float resultado;
    // Copia la representación binaria de 'bits' en la variable float 'resultado'
    std::memcpy(&resultado, &bits, sizeof(float));
    return resultado;
}

// Función que implementa la división manual de números flotantes.
float dividir(float a, float b) {
    // Caso especial: división por cero.
    if (b == 0.0f) {
        // Si a es 0, se retorna NaN (not a number).
        if (a == 0.0f)
            return NAN;
        // Si b es 0 pero a no lo es, se retorna infinito.
        // Se determina el signo del resultado: se usa 0 para positivo y 1 para negativo.
        // Si a > 0 se considera resultado positivo, en caso contrario negativo.
        return (a > 0.0f) ? componer(0, 0xFF, 0) : componer(1, 0xFF, 0);
    }
    // Si el numerador es 0, el resultado de la división es 0.
    if (a == 0.0f)
        return 0.0f;
    
    // Variables para almacenar cada parte del número 'a'
    uint32_t signoA, expA, parteA;
    // Variables para almacenar cada parte del número 'b'
    uint32_t signoB, expB, parteB;
    
    // Descomponemos 'a' y 'b' en sus componentes (signo, exponente y parteSignificativa)
    descomponer(a, signoA, expA, parteA);
    descomponer(b, signoB, expB, parteB);
    
    // Se calcula el signo del resultado.
    // Si los signos son iguales, el resultado es positivo (0); si son distintos, es negativo (1)
    uint32_t signoR = signoA ^ signoB;
    
    // Se calcula el exponente del resultado.
    // Se remueve el bias (127) de cada exponente, se realiza la resta de los mismos, y luego se vuelve a agregar el bias.
    int32_t expR = (int32_t(expA) - 127) - (int32_t(expB) - 127) + 127;
    
    // Se verifican los casos de sobreflujo o subflujo del exponente.
    if (expR >= 0xFF)
        return componer(signoR, 0xFF, 0);  // Se retorna infinito si hay sobreflujo.
    if (expR <= 0)
        return componer(signoR, 0, 0);       // Retorna cero si hay subflujo (aquí se podría mejorar el manejo de números subnormales).
    
    // División de las partes significativas:
    // Se añade el bit implícito '1' al comienzo (resultando en un valor de 24 bits), ya que en la representación IEEE 754
    // los números normalizados siempre tienen un 1 implícito a la izquierda del punto binario.
    // Se desplaza la parteSignificativa de 'a' 23 posiciones a la izquierda para preservar la precisión antes de dividir.
    uint64_t numerador = (0x800000ULL | parteA) << 23;
    uint64_t denominador = (0x800000ULL | parteB);
    // Se divide el numerador entre el denominador para obtener la parte significativa del resultado.
    uint64_t parteRes = numerador / denominador;
    
    // Normalización del resultado: la parteRes debe estar en el rango [1.0, 2.0), lo que significa que
    // el bit 23 (de 0 a 23, siendo 24 bits en total) debe estar encendido.
    // Si la parteRes es mayor o igual a 2.0 (es decir, tiene un 1 en el bit 24) se realiza un corrimiento a la derecha y se incrementa el exponente.
    if (parteRes >= (1ULL << 24)) {
        parteRes >>= 1;
        expR++;
    }
    // Si la parteRes está por debajo de 1.0 (el bit 23 no está encendido), se corrige desplazándola a la izquierda y
    // se decrementa el exponente.
    else if (!(parteRes & (1ULL << 23))) {
        parteRes <<= 1;
        expR--;
    }
    
    // Se verifica nuevamente que el exponente no se haya salido de rango después de la normalización.
    if (expR >= 0xFF)
        return componer(signoR, 0xFF, 0);  // Infinito
    if (expR <= 0)
        return componer(signoR, 0, 0);       // Cero (o subnormal, que aquí se simplifica a 0)
    
    // Se elimina el bit implícito (se guarda solo los 23 bits inferiores) al recomponer el número.
    return componer(signoR, static_cast<uint32_t>(expR), static_cast<uint32_t>(parteRes & 0x7FFFFF));
}

int main() {
    float a = 30.0f, b = 1.5f;
    // Se muestra por pantalla el resultado de la división manual.
    std::cout << "División: " << dividir(a, b) << std::endl;  // Se espera 5.0
    return 0;
}
