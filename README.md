# MapReduce Implementation

Pentru implementarea acestei teme mi-am creat o clasa (`MapReduce`) in cadrul careia retin toate variabilele necesare thread-urilor. In constructorul acestei clase, imi pornesc cele `M + R` thread-uri care primesc ca argumente ID-ul thread-ului respectiv si instanta clasei (toate thread-urile au ca referinta o singura instanta).

Thread-urile vor fi apoi triate pe baza ID-ului in functia `thread_func`. Astfel, cele cu `ID < M` vor executa prima parte a functiei (<u>Mappers</u>), iar cele cu `ID >= M` (<u>Reducers</u>) vor executa partea ramasa.

---

## Thread-urile Mappers

Din lista de fisiere, un **singur** thread Mapper isi poate alege un index la orice moment de timp. 

1. Dupa ce si-a ales indexul, deblocheaza accesul in zona critica si incepe sa prelucreze fisierul. 
2. Din fisier va sterge:
   - caracterele speciale,
   - spatiile consecutive
   - si va transforma textul in lowercase. 
3. Fiecare thread Mapper va salva cuvintele intr-o lista de liste (`vector<unordered_map>`), mai exact in lista cu indexul fisierului primit.

---

## Se asteapta executia tuturor thread-urilor Mapper.


## Thread-urile Reducer

Stiind ca vor exista 26 (`a-z`) de liste finale, fiecare thread Reducer isi va alege o litera disponibila pornind de la indexul `0`. 

- La fel ca in cazul Mappers, **un singur thread Reducer poate primi un index la orice moment de timp**. 
- Dupa aceea, fiecare thread va cauta prin toate listele primite de la thread-urile Mapper pentru a gasi cuvinte care incep cu litera primita.
- **Aceasta sectiune nu necesita sincronizare**, deoarece thread-urile doar citesc date.
- Dupa aceea, cuvintele vor fi sortate si scrise in fisier de fiecare thread Reducer in parte.

---


### Atat pentru thread-urile Mapper cat si pentru cele Reducer, alocarea sarcinilor se face **dinamic**. 
 - In momentul in care un thread termina, acesta trece la urmatoarea lista sau fisier de prelucrat. 
Acest lucru previne situatiile in care un thread nu are ce sa faca, in timp ce altul inca lucreaza.
