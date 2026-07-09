
#include "turan/neighbours.h"
#include <iostream>

//Implementación basica de neighbours basada en el primer paper y su versión sin estructuras adicionales
//Complejidad : O(min(degree(nodo1),degree(nodo2)))
bool neighbours(pemb<>& graph, int nodo1, int nodo2){
        pemb<>::size_type vertices = graph.vertices();
		if (nodo1 >= vertices || nodo2 >= vertices)
			return false;

		pemb<>::size_type nxt_nodo1 = graph.first(nodo1);
        pemb<>::size_type nxt_nodo2 = graph.first(nodo2);

        pemb<>::size_type edges = graph.edges();
        pemb<>::size_type mt ;
		while (nxt_nodo1 < 2 * edges && nxt_nodo2 < 2 * edges)
		{   

			mt = graph.mate(nxt_nodo1);

			if(graph.vertex(mt) == nodo2){
                return true;
            } 

            mt = graph.mate(nxt_nodo2);

            if (graph.vertex(mt) == nodo1){
               return true;
            }
            
			nxt_nodo1 = graph.next(nxt_nodo1);
            nxt_nodo2 = graph.next(nxt_nodo2);

        }
        return false;
		
}