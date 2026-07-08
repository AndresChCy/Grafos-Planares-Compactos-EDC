#pragma once

#include "sdsl/pemb.hpp"

//Implementación basica de neighbours basada en el primer paper y su versión sin estructuras adicionales
//Complejidad : O(min(degree(nodo1),degree(nodo2)))
bool neighbours(pemb<>& graph, int nodo1, int nodo2);