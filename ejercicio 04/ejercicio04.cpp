#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <string>
#include <tuple>

using namespace std;
using namespace chrono;

const int INF = 1e9;

// ESTRUCTURAS
struct Arista {
    int u, v, peso;
};

struct Resultado {
    vector<Arista> aristas;
    long long pesoTotal = 0;
    bool conectado = false;
    double tiempoMs = 0;
    vector<string> recorrido;
};

struct Grafo {
    int n;
    vector<Arista> aristas;
    vector<vector<pair<int, int>>> lista;
    vector<vector<int>> matriz;

    Grafo(int vertices) {
        n = vertices;
        lista.resize(n);
        matriz.assign(n, vector<int>(n, 0));
    }

    void agregarArista(int u, int v, int peso) {
        aristas.push_back({u, v, peso});

        lista[u].push_back({v, peso});
        lista[v].push_back({u, peso});

        matriz[u][v] = peso;
        matriz[v][u] = peso;
    }
};

// GENERACION DEL GRAFO ALEATORIO

Grafo generarGrafo(int n, mt19937& rng) {
    Grafo g(n);

    uniform_int_distribution<int> peso(1, 100);

    // Crear un arbol para garantizar conectividad.
    for (int v = 1; v < n; v++) {
        uniform_int_distribution<int> anterior(0, v - 1);
        int u = anterior(rng);

        g.agregarArista(u, v, peso(rng));
    }

    // Agregar conexiones aleatorias adicionales.
    uniform_int_distribution<int> probabilidad(1, 100);

    for (int u = 0; u < n; u++) {
        for (int v = u + 1; v < n; v++) {
            if (g.matriz[u][v] == 0 &&
                probabilidad(rng) <= 15) {
                g.agregarArista(u, v, peso(rng));
            }
        }
    }

    return g;
}

// UNION-FIND: UTILIZADO POR KRUSKAL
struct UnionFind {
    vector<int> padre, rango;

    UnionFind(int n) {
        padre.resize(n);
        rango.assign(n, 0);

        iota(padre.begin(), padre.end(), 0);
    }

    int encontrar(int x) {
        if (padre[x] != x)
            padre[x] = encontrar(padre[x]);

        return padre[x];
    }

    bool unir(int a, int b) {
        a = encontrar(a);
        b = encontrar(b);

        if (a == b)
            return false;

        if (rango[a] < rango[b])
            swap(a, b);

        padre[b] = a;

        if (rango[a] == rango[b])
            rango[a]++;

        return true;
    }
};

// KRUSKAL CON LISTA DE ADYACENCIA
Resultado kruskalLista(const Grafo& g, bool trazar = false) {
    vector<Arista> aristas;

    // Extraer cada arista una sola vez.
    for (int u = 0; u < g.n; u++) {
        for (auto [v, peso] : g.lista[u]) {
            if (u < v)
                aristas.push_back({u, v, peso});
        }
    }

    // Ordenar por peso ascendente.
    sort(aristas.begin(), aristas.end(),
         [](const Arista& a, const Arista& b) {
             return a.peso < b.peso;
         });

    UnionFind uf(g.n);
    Resultado r;

    if (trazar) {
        r.recorrido.push_back(
            "Se ordenan las aristas por peso ascendente."
        );
    }

    for (const auto& a : aristas) {
        bool aceptada = uf.unir(a.u, a.v);

        if (trazar) {
            ostringstream paso;

            paso << "(" << a.u << " - " << a.v
                 << ", peso " << a.peso << ") -> "
                 << (aceptada
                     ? "ACEPTADA"
                     : "RECHAZADA: forma un ciclo");

            r.recorrido.push_back(paso.str());
        }

        if (aceptada) {
            r.aristas.push_back(a);
            r.pesoTotal += a.peso;

            if ((int)r.aristas.size() == g.n - 1)
                break;
        }
    }

    r.conectado = (g.n == 1 ||
                   (int)r.aristas.size() == g.n - 1);

    return r;
}

// KRUSKAL CON MATRIZ DE ADYACENCIA
Resultado kruskalMatriz(const Grafo& g, bool trazar = false) {
    vector<Arista> aristas;

    // Extraer las aristas de la matriz.
    for (int u = 0; u < g.n; u++) {
        for (int v = u + 1; v < g.n; v++) {
            if (g.matriz[u][v] != 0) {
                aristas.push_back(
                    {u, v, g.matriz[u][v]}
                );
            }
        }
    }

    sort(aristas.begin(), aristas.end(),
         [](const Arista& a, const Arista& b) {
             return a.peso < b.peso;
         });

    UnionFind uf(g.n);
    Resultado r;

    if (trazar) {
        r.recorrido.push_back(
            "Se ordenan las aristas por peso ascendente."
        );
    }

    for (const auto& a : aristas) {
        bool aceptada = uf.unir(a.u, a.v);

        if (trazar) {
            ostringstream paso;

            paso << "(" << a.u << " - " << a.v
                 << ", peso " << a.peso << ") -> "
                 << (aceptada
                     ? "ACEPTADA"
                     : "RECHAZADA: forma un ciclo");

            r.recorrido.push_back(paso.str());
        }

        if (aceptada) {
            r.aristas.push_back(a);
            r.pesoTotal += a.peso;

            if ((int)r.aristas.size() == g.n - 1)
                break;
        }
    }

    r.conectado = (g.n == 1 ||
                   (int)r.aristas.size() == g.n - 1);

    return r;
}

// PRIM CON LISTA DE ADYACENCIA
Resultado primLista(const Grafo& g, bool trazar = false) {
    Resultado r;

    vector<bool> visitado(g.n, false);

    // peso, vertice actual, vertice padre
    using Nodo = tuple<int, int, int>;

    priority_queue<Nodo, vector<Nodo>,
                   greater<Nodo>> pq;

    // Empezar por el vertice 0.
    pq.push({0, 0, -1});

    if (trazar) {
        r.recorrido.push_back(
            "Se inicia en el vertice 0."
        );
    }

    while (!pq.empty()) {
        auto [peso, u, padre] = pq.top();
        pq.pop();

        if (visitado[u])
            continue;

        visitado[u] = true;

        if (padre != -1) {
            r.aristas.push_back({padre, u, peso});
            r.pesoTotal += peso;

            if (trazar) {
                ostringstream paso;

                paso << "Se agrega la arista ("
                     << padre << " - " << u
                     << ", peso " << peso << ").";

                r.recorrido.push_back(paso.str());
            }
        } else if (trazar) {
            r.recorrido.push_back(
                "Se visita el vertice inicial 0."
            );
        }

        // Explorar vecinos del vertice seleccionado.
        for (auto [v, costo] : g.lista[u]) {
            if (!visitado[v]) {
                pq.push({costo, v, u});
            }
        }
    }

    r.conectado = (g.n == 1 ||
                   (int)r.aristas.size() == g.n - 1);

    return r;
}

// PRIM CON MATRIZ DE ADYACENCIA
Resultado primMatriz(const Grafo& g, bool trazar = false) {
    Resultado r;

    vector<int> clave(g.n, INF);
    vector<int> padre(g.n, -1);
    vector<bool> visitado(g.n, false);

    clave[0] = 0;

    if (trazar) {
        r.recorrido.push_back(
            "Se inicia en el vertice 0."
        );
    }

    for (int i = 0; i < g.n; i++) {
        int u = -1;

        // Buscar el vertice no visitado con menor clave.
        for (int v = 0; v < g.n; v++) {
            if (!visitado[v] &&
                (u == -1 || clave[v] < clave[u])) {
                u = v;
            }
        }

        if (u == -1 || clave[u] == INF)
            break;

        visitado[u] = true;

        if (padre[u] != -1) {
            r.aristas.push_back(
                {padre[u], u, clave[u]}
            );

            r.pesoTotal += clave[u];

            if (trazar) {
                ostringstream paso;

                paso << "Se agrega la arista ("
                     << padre[u] << " - " << u
                     << ", peso " << clave[u] << ").";

                r.recorrido.push_back(paso.str());
            }
        } else if (trazar) {
            r.recorrido.push_back(
                "Se visita el vertice inicial 0."
            );
        }

        // Actualizar los pesos de los vecinos.
        for (int v = 0; v < g.n; v++) {
            int peso = g.matriz[u][v];

            if (peso != 0 &&
                !visitado[v] &&
                peso < clave[v]) {
                clave[v] = peso;
                padre[v] = u;
            }
        }
    }

    r.conectado = (g.n == 1 ||
                   (int)r.aristas.size() == g.n - 1);

    return r;
}

// MEDICION DEL TIEMPO
template <typename Funcion>
Resultado medirTiempo(Funcion funcion) {
    auto inicio = steady_clock::now();

    Resultado r = funcion();

    auto fin = steady_clock::now();

    r.tiempoMs =
        duration<double, milli>(fin - inicio).count();

    return r;
}

// MOSTRAR RESULTADOS
void mostrarAristas(const Resultado& r) {
    for (const auto& a : r.aristas) {
        cout << "(" << a.u << " - " << a.v
             << ", peso " << a.peso << ") ";
    }

    cout << "\n";
}

void mostrarResultado(const string& nombre,
                      const Resultado& r) {
    cout << "\n" << nombre << "\n";

    cout << "Aristas del arbol: "
         << r.aristas.size() << "\n";

    cout << "Peso total: "
         << r.pesoTotal << "\n";

    cout << "Tiempo: "
         << fixed << setprecision(6)
         << r.tiempoMs << " ms\n";

    cout << "Arbol completo: "
         << (r.conectado ? "Si" : "No") << "\n";
}

// MOSTRAR LISTA DE ADYACENCIA DEL GRAFO ORIGINAL
void mostrarListaAdyacencia(const Grafo& g) {
    cout << "\n—-------------- LISTA DE ADYACENCIA —------------\n";
    for (int u = 0; u < g.n; u++) {
        cout << "Vertice " << u << " -> [";

        for (int i = 0; i < (int)g.lista[u].size(); i++) {
            auto [v, peso] = g.lista[u][i];

            cout << v << " (peso " << peso << ")";

            if (i + 1 < (int)g.lista[u].size())
                cout << ", ";
        }

        cout << "]\n";
    }
}

// MOSTRAR MATRIZ DE ADYACENCIA DEL GRAFO ORIGINAL
void mostrarMatrizAdyacencia(const Grafo& g) {
    cout << "\n------------------ MATRIZ DE ADYACENCIA -----------------\n";

    cout << setw(5) << " ";

    for (int j = 0; j < g.n; j++)
        cout << setw(5) << j;

    cout << "\n";

    for (int i = 0; i < g.n; i++) {
        cout << setw(5) << i;

        for (int j = 0; j < g.n; j++)
            cout << setw(5) << g.matriz[i][j];

        cout << "\n";
    }

    cout << "Nota: 0 significa que no existe una arista.\n";
}

// MOSTRAR RECORRIDO DE CADA ALGORITMO
void mostrarRecorrido(const string& nombre,
                      const Resultado& r) {
    cout << "\n------------- RECORRIDO: "
         << nombre << " ---------------\n";

    for (int i = 0; i < (int)r.recorrido.size(); i++) {
        cout << i + 1 << ". "
             << r.recorrido[i] << "\n";
    }

    cout << "\nAristas seleccionadas del AEM:\n";
    mostrarAristas(r);

    cout << "Peso total del AEM: "
         << r.pesoTotal << "\n";
}

// PROGRAMA PRINCIPAL
int main() {
    random_device rd;
    mt19937 rng(rd());

    int n;

    cout << "Ingrese la cantidad de vertices: ";
    cin >> n;

    if (cin.fail() || n < 1 || n > 500) {
        cout << "Ingrese un numero entre 1 y 500.\n";
        return 1;
    }

    // Generar un unico grafo para los cuatro algoritmos.
    Grafo g = generarGrafo(n, rng);

    cout << "\n--------------- GRAFO GENERADO --------------\n";
    cout << "Vertices: " << n << "\n";
    cout << "Aristas: " << g.aristas.size() << "\n";
    cout << "Pesos aleatorios: entre 1 y 100\n";

    // MEDIR TIEMPOS SIN IMPRIMIR NI REGISTRAR RECORRIDOS
    Resultado kl = medirTiempo([&]() {
        return kruskalLista(g);
    });

    Resultado km = medirTiempo([&]() {
        return kruskalMatriz(g);
    });

    Resultado pl = medirTiempo([&]() {
        return primLista(g);
    });

    Resultado pm = medirTiempo([&]() {
        return primMatriz(g);
    });

    // RESULTADOS
    cout << "\n--------------- RESULTADOS ----------------\n";

    mostrarResultado(
        "1. Kruskal - Lista de adyacencia", kl
    );

    mostrarResultado(
        "2. Kruskal - Matriz de adyacencia", km
    );

    mostrarResultado(
        "3. Prim - Lista de adyacencia", pl
    );

    mostrarResultado(
        "4. Prim - Matriz de adyacencia", pm
    );

    // COMPARACION DE RESULTADOS
    cout << "\n---------------- COMPARACION ----------------\n";

    bool correctos = kl.conectado &&
                     km.conectado &&
                     pl.conectado &&
                     pm.conectado;

    bool mismoPeso = kl.pesoTotal == km.pesoTotal &&
                     kl.pesoTotal == pl.pesoTotal &&
                     kl.pesoTotal == pm.pesoTotal;

    if (correctos && mismoPeso) {
        cout << "Correcto: las cuatro implementaciones "
             << "obtuvieron un AEM con el mismo peso minimo.\n";
    } else {
        cout << "Advertencia: revisar las implementaciones "
             << "o la conectividad del grafo.\n";
    }

    cout << "\nComparacion de tiempos:\n";

    cout << "Kruskal - Lista:  "
         << kl.tiempoMs << " ms\n";

    cout << "Kruskal - Matriz: "
         << km.tiempoMs << " ms\n";

    cout << "Prim - Lista:     "
         << pl.tiempoMs << " ms\n";

    cout << "Prim - Matriz:    "
         << pm.tiempoMs << " ms\n";

    // REPRESENTACIONES DEL GRAFO ORIGINAL: UNA SOLA VEZ
    mostrarListaAdyacencia(g);
    mostrarMatrizAdyacencia(g);

    // CALCULAR RECORRIDOS FUERA DE LA MEDICION
    Resultado klPasos = kruskalLista(g, true);
    Resultado kmPasos = kruskalMatriz(g, true);

    Resultado plPasos = primLista(g, true);
    Resultado pmPasos = primMatriz(g, true);

    // MOSTRAR RECORRIDOS
    cout << "\n----------------- RECORRIDOS DE LOS ALGORITMOS ---------------n";

    mostrarRecorrido(
        "KRUSKAL - LISTA DE ADYACENCIA", klPasos
    );

    mostrarRecorrido(
        "KRUSKAL - MATRIZ DE ADYACENCIA", kmPasos
    );

    mostrarRecorrido(
        "PRIM - LISTA DE ADYACENCIA", plPasos
    );

    mostrarRecorrido(
        "PRIM - MATRIZ DE ADYACENCIA", pmPasos
    );

    return 0;
}
