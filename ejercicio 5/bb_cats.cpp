#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;
using namespace chrono;

const int INF = 1e9;

struct Arista {
    int u, v, km;
};

struct Resultado {
    vector<Arista> aristas;
    vector<int> orden;
    int kmTotal = 0;
    int procesadas = 0;
};

vector<string> nombres = {"Base", "Refugio", "Veterinaria", "Tienda",
                          "Adopciones", "Casa Acogida", "Albergue", "Clinica"};

vector<Arista> rutasBB = {{0, 1, 4}, {0, 2, 2}, {0, 3, 7}, {1, 2, 3}, {1, 4, 5}, {1, 6, 10},
                          {2, 3, 6}, {2, 4, 1}, {3, 4, 8}, {3, 5, 4}, {3, 7, 11}, {4, 5, 9},
                          {4, 6, 6}, {5, 6, 3}, {5, 7, 5}, {6, 7, 7}};

struct UnionFind {
    vector<int> padre, rango;

    UnionFind(int n) {
        padre.resize(n);
        rango.assign(n, 0);
        for (int i = 0; i < n; i++) padre[i] = i;
    }

    int encontrar(int x) {
        if (padre[x] != x) padre[x] = encontrar(padre[x]);
        return padre[x];
    }

    bool unir(int a, int b) {
        a = encontrar(a);
        b = encontrar(b);
        if (a == b) return false;
        if (rango[a] < rango[b]) swap(a, b);
        padre[b] = a;
        if (rango[a] == rango[b]) rango[a]++;
        return true;
    }
};

vector<vector<int>> crearMatriz(int n, const vector<Arista>& rutas) {
    vector<vector<int>> matriz(n, vector<int>(n, 0));
    for (Arista a : rutas) matriz[a.u][a.v] = matriz[a.v][a.u] = a.km;
    return matriz;
}

Resultado kruskal(int n, vector<Arista> rutas, bool mostrar) {
    sort(rutas.begin(), rutas.end(), [](Arista a, Arista b) { return a.km < b.km; });

    UnionFind uf(n);
    Resultado r;
    vector<bool> enRed(n, false);

    for (Arista a : rutas) {
        r.procesadas++;
        bool aceptada = uf.unir(a.u, a.v);
        if (mostrar)
            cout << "  " << left << setw(27) << nombres[a.u] + " - " + nombres[a.v] << right << setw(3)
                 << a.km << " km   " << (aceptada ? "HABILITADA" : "DESCARTADA") << "\n";
        if (aceptada) {
            r.aristas.push_back(a);
            r.kmTotal += a.km;
            if (!enRed[a.u]) r.orden.push_back(a.u);
            if (!enRed[a.v]) r.orden.push_back(a.v);
            enRed[a.u] = enRed[a.v] = true;
            if ((int)r.aristas.size() == n - 1) break;
        }
    }
    return r;
}

Resultado prim(int n, const vector<vector<int>>& matriz, int inicio, bool mostrar) {
    vector<int> clave(n, INF), padre(n, -1);
    vector<bool> visitado(n, false);
    Resultado r;
    clave[inicio] = 0;

    for (int i = 0; i < n; i++) {
        int u = -1;
        for (int v = 0; v < n; v++)
            if (!visitado[v] && (u == -1 || clave[v] < clave[u])) u = v;

        visitado[u] = true;
        r.orden.push_back(u);
        if (padre[u] != -1) {
            r.aristas.push_back({padre[u], u, clave[u]});
            r.kmTotal += clave[u];
            if (mostrar)
                cout << "  Paso " << r.aristas.size() << ": " << left << setw(13) << nombres[padre[u]]
                     << "-> " << setw(13) << nombres[u] << right << setw(3) << clave[u] << " km\n";
        }

        for (int v = 0; v < n; v++) {
            if (matriz[u][v] == 0) continue;
            r.procesadas++;
            if (!visitado[v] && matriz[u][v] < clave[v]) {
                clave[v] = matriz[u][v];
                padre[v] = u;
            }
        }
    }
    return r;
}

double cronometrar(int n, const vector<Arista>& rutas, const vector<vector<int>>& matriz,
                   bool esKruskal, int inicio, int reps) {
    auto t0 = steady_clock::now();
    for (int i = 0; i < reps; i++) {
        if (esKruskal) kruskal(n, rutas, false);
        else prim(n, matriz, inicio, false);
    }
    auto t1 = steady_clock::now();
    return duration<double, micro>(t1 - t0).count() / reps;
}

vector<Arista> generarRutas(int n, double probabilidad) {
    mt19937 rng(2026);
    uniform_int_distribution<int> km(1, 100);
    uniform_real_distribution<double> azar(0.0, 1.0);
    vector<vector<bool>> existe(n, vector<bool>(n, false));
    vector<Arista> rutas;

    for (int v = 1; v < n; v++) {
        uniform_int_distribution<int> anterior(0, v - 1);
        int u = anterior(rng);
        rutas.push_back({u, v, km(rng)});
        existe[u][v] = true;
    }
    for (int u = 0; u < n; u++)
        for (int v = u + 1; v < n; v++)
            if (!existe[u][v] && azar(rng) < probabilidad) rutas.push_back({u, v, km(rng)});
    return rutas;
}

void titulo(string texto, string subtitulo) {
    cout << "\n" << string(70, '=') << "\n  " << texto << "\n" << string(70, '=') << "\n";
    cout << subtitulo << "\n\n";
}

void mostrarOrden(string algoritmo, const Resultado& r) {
    cout << algoritmo << "\n  ";
    for (int i = 0; i < (int)r.orden.size(); i++) {
        cout << nombres[r.orden[i]];
        if (i + 1 < (int)r.orden.size()) cout << (i % 4 == 3 ? " ->\n  " : " -> ");
    }
    cout << "\n  Total de la red: " << r.kmTotal << " km\n\n";
}

void tablaComparacion(const Resultado& k, const Resultado& p, double tk, double tp, string nombrePrim) {
    cout << string(70, '-') << "\n";
    cout << left << setw(32) << "Criterio" << setw(14) << "Kruskal" << nombrePrim << "\n";
    cout << string(70, '-') << "\n";
    cout << setw(32) << "Km de la red" << setw(14) << to_string(k.kmTotal) + " km" << p.kmTotal << " km\n";
    cout << setw(32) << "Rutas habilitadas" << setw(14) << k.aristas.size() << p.aristas.size() << "\n";
    cout << setw(32) << "Aristas procesadas" << setw(14) << k.procesadas << p.procesadas << "\n";
    cout << fixed << setprecision(3);
    cout << setw(32) << "Microsegundos por ejecucion" << setw(14) << tk << tp << "\n";
    cout << string(70, '-') << "\n" << right;
}

int main() {
    int n = nombres.size();
    vector<vector<int>> matriz = crearMatriz(n, rutasBB);

    titulo("BB CATS - TRANSPORTE DE GATITOS RESCATADOS",
           "Objetivo: conectar las " + to_string(n) + " sedes habilitando el menor total de km de rutas,\n"
           "es decir, encontrar el arbol de expansion minima de la red.");

    titulo("MATRIZ DE ADYACENCIA",
           "Distancias en km entre cada par de sedes. Un guion indica que no hay ruta directa.\n"
           "Prim trabaja con esta matriz; Kruskal usa la lista de las " + to_string(rutasBB.size()) + " rutas.");
    cout << setw(17) << " ";
    for (int j = 0; j < n; j++) cout << setw(4) << j + 1;
    cout << "\n";
    for (int i = 0; i < n; i++) {
        cout << "  " << i + 1 << ". " << left << setw(12) << nombres[i] << right;
        for (int j = 0; j < n; j++) {
            if (matriz[i][j] == 0) cout << setw(4) << "-";
            else cout << setw(4) << matriz[i][j];
        }
        cout << "\n";
    }

    titulo("KRUSKAL: LISTA DE ARISTAS + UNION-FIND",
           "Revisa las rutas de la mas corta a la mas larga. Una ruta se descarta si sus\n"
           "dos sedes ya estaban conectadas, porque formaria un ciclo.");
    Resultado k = kruskal(n, rutasBB, true);

    int inicio;
    cout << "\nDesde que sede parte Prim? (1 a " << n << "): ";
    while (!(cin >> inicio) || inicio < 1 || inicio > n) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Numero invalido. Ingrese un numero del 1 al " << n << ": ";
    }
    inicio--;

    titulo("PRIM: MATRIZ DE ADYACENCIA, DESDE " + nombres[inicio],
           "La red crece desde " + nombres[inicio] + ". En cada paso se conecta la sede mas cercana\n"
           "a las que ya estan en la red, revisando su fila de la matriz.");
    Resultado p = prim(n, matriz, inicio, true);

    titulo("ORDEN EN QUE SE CONECTARON LAS SEDES",
           "Secuencia en que cada algoritmo fue agregando las sedes a la red.");
    mostrarOrden("Kruskal:", k);
    mostrarOrden("Prim desde " + nombres[inicio] + ":", p);

    const int REPS_BB = 100000;
    double tk = cronometrar(n, rutasBB, matriz, true, inicio, REPS_BB);
    double tp = cronometrar(n, rutasBB, matriz, false, inicio, REPS_BB);
    titulo("COMPARACION EN LA RED DE BB CATS",
           to_string(n) + " sedes y " + to_string(rutasBB.size()) + " rutas. Tiempo: promedio de " +
               to_string(REPS_BB) + " ejecuciones de cada algoritmo.");
    tablaComparacion(k, p, tk, tp, "Prim desde " + nombres[inicio]);

    const int N_PRUEBA = 100, REPS_PRUEBA = 2000;
    vector<Arista> rutasPrueba = generarRutas(N_PRUEBA, 0.3);
    vector<vector<int>> matrizPrueba = crearMatriz(N_PRUEBA, rutasPrueba);
    Resultado kp = kruskal(N_PRUEBA, rutasPrueba, false);
    Resultado pp = prim(N_PRUEBA, matrizPrueba, 0, false);
    double tkp = cronometrar(N_PRUEBA, rutasPrueba, matrizPrueba, true, 0, REPS_PRUEBA);
    double tpp = cronometrar(N_PRUEBA, rutasPrueba, matrizPrueba, false, 0, REPS_PRUEBA);
    titulo("CASO DE PRUEBA: RED GRANDE DE " + to_string(N_PRUEBA) + " SEDES",
           "Red generada al azar con " + to_string(rutasPrueba.size()) + " rutas de " +
               to_string(N_PRUEBA * (N_PRUEBA - 1) / 2) + " posibles y distancias de 1 a 100 km,\n"
               "para ver como cambia el tiempo con muchas sedes. Prim parte de la sede 1.\n"
               "Tiempo: promedio de " + to_string(REPS_PRUEBA) + " ejecuciones de cada algoritmo.");
    tablaComparacion(kp, pp, tkp, tpp, "Prim desde sede 1");

    return 0;
}
