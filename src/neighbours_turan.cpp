#include "sdsl/pemb.hpp"


//Implementación basica de neighbours basada en el primer paper y su versión sin estructuras adicionales
//Complejidad : O(min(degree(nodo1),degree(nodo2)))
bool neighbours(pemb<> graph, int nodo1, int nodo2){
   
		if (nodo1 >= graph.vertices()|| nodo2 >= graph.vertices() )
			return false;

		pemb<>::size_type nxt_nodo1 = graph.first(nodo1);
        pemb<>::size_type nxt_nodo2 = graph.first(nodo2);

		while (nxt_nodo1 < 2 * graph.edges() && nxt_nodo2 < 2 * graph.edges())
		{
			if (nxt_nodo1 < 2 *graph.edges())
			{
				pemb<>::size_type mt = graph.mate(nxt_nodo1);
				if(graph.vertex(mt) == nodo2){
                    return true;
                } 
			}
            if (nxt_nodo2 < 2 *graph.edges())
            {
                pemb<>::size_type mt = graph.mate(nxt_nodo2);
                if (graph.vertex(mt) == nodo2){
                    return true;
                }
            }
			nxt_nodo1 = graph.next(nxt_nodo1);
            nxt_nodo2 = graph.next(nxt_nodo2);
        }
        return false;
		
}