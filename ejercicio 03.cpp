// Ejercicio 3 - Lab 4 ADA: Prim vs Kruskal en grafos aleatorios (10, 50, 100 vertices)
#include <bits/stdc++.h>
using namespace std;
using namespace std::chrono;

struct Arista { int u, v, w; };

// ---------- Generador de grafos aleatorios conexos ----------
vector<Arista> generarGrafo(int n, double p, mt19937 &rng) {
    uniform_int_distribution<int> peso(1, 100);
    uniform_real_distribution<double> prob(0.0, 1.0);
    set<pair<int,int>> usadas;
    vector<Arista> E;

    vector<int> orden(n);
    iota(orden.begin(), orden.end(), 0);
    shuffle(orden.begin(), orden.end(), rng);
    for (int i = 1; i < n; i++) {
        uniform_int_distribution<int> padre(0, i - 1);
        int a = orden[i], b = orden[padre(rng)];
        if (a > b) swap(a, b);
        usadas.insert({a, b});
        E.push_back({a, b, peso(rng)});
    }
    for (int a = 0; a < n; a++)
        for (int b = a + 1; b < n; b++)
            if (!usadas.count({a, b}) && prob(rng) < p) {
                usadas.insert({a, b});
                E.push_back({a, b, peso(rng)});
            }
    return E;
}

// ---------- Kruskal (ordenamiento + Union-Find) ----------
struct UnionFind {
    vector<int> padre, rango;
    UnionFind(int n) : padre(n), rango(n, 0) { iota(padre.begin(), padre.end(), 0); }
    int find(int x) { return padre[x] == x ? x : padre[x] = find(padre[x]); }
    bool unir(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;          
        if (rango[a] < rango[b]) swap(a, b);
        padre[b] = a;
        if (rango[a] == rango[b]) rango[a]++;
        return true;
    }
};

vector<Arista> kruskal(int n, vector<Arista> E) {
    sort(E.begin(), E.end(), [](const Arista &x, const Arista &y) { return x.w < y.w; });
    UnionFind uf(n);
    vector<Arista> T;
    for (auto &e : E) {
        if (uf.unir(e.u, e.v)) T.push_back(e);
        if ((int)T.size() == n - 1) break;
    }
    return T;
}

// ---------- Prim (lista de adyacencia + min-heap) ----------
vector<Arista> prim(int n, const vector<vector<pair<int,int>>> &adj, int inicio = 0) {
    vector<bool> visitado(n, false);
    priority_queue<tuple<int,int,int>, vector<tuple<int,int,int>>, greater<>> pq; 
    vector<Arista> T;
    visitado[inicio] = true;
    for (auto [v, w] : adj[inicio]) pq.push({w, inicio, v});
    while (!pq.empty() && (int)T.size() < n - 1) {
        auto [w, u, v] = pq.top(); pq.pop();
        if (visitado[v]) continue;        
        visitado[v] = true;
        T.push_back({u, v, w});
        for (auto [x, wx] : adj[v]) if (!visitado[x]) pq.push({wx, v, x});
    }
    return T;
}

// ---------- Utilidades ----------
long long costo(const vector<Arista> &T) {
    long long s = 0; for (auto &e : T) s += e.w; return s;
}
set<tuple<int,int,int>> normalizar(const vector<Arista> &T) {
    set<tuple<int,int,int>> s;
    for (auto &e : T) s.insert({min(e.u, e.v), max(e.u, e.v), e.w});
    return s;
}

int main() {
    mt19937 rng(2026);                     
    const double P = 0.3;                 
    const int REPS = 1000;                 
    vector<int> tamanos = {10, 50, 100};

    cout << left << setw(6) << "V" << setw(7) << "E"
         << setw(16) << "Prim (us)" << setw(16) << "Kruskal (us)"
         << setw(12) << "Costo P" << setw(12) << "Costo K"
         << setw(14) << "Mismo costo" << "Mismas aristas\n";
    cout << string(95, '-') << "\n";

    for (int n : tamanos) {
        auto E = generarGrafo(n, P, rng);
        vector<vector<pair<int,int>>> adj(n);
        for (auto &e : E) { adj[e.u].push_back({e.v, e.w}); adj[e.v].push_back({e.u, e.w}); }

        vector<Arista> Tp, Tk;
        auto t0 = high_resolution_clock::now();
        for (int r = 0; r < REPS; r++) Tp = prim(n, adj, 0);
        auto t1 = high_resolution_clock::now();
        for (int r = 0; r < REPS; r++) Tk = kruskal(n, E);
        auto t2 = high_resolution_clock::now();

        double tp = duration<double, micro>(t1 - t0).count() / REPS;
        double tk = duration<double, micro>(t2 - t1).count() / REPS;
        bool mismoCosto = costo(Tp) == costo(Tk);
        bool mismasAristas = normalizar(Tp) == normalizar(Tk);

        cout << setw(6) << n << setw(7) << E.size()
             << setw(16) << fixed << setprecision(3) << tp
             << setw(16) << tk
             << setw(12) << costo(Tp) << setw(12) << costo(Tk)
             << setw(14) << (mismoCosto ? "Si" : "No")
             << (mismasAristas ? "Si" : "No") << "\n";

        if (!mismasAristas) {
            auto sp = normalizar(Tp), sk = normalizar(Tk);
            int dif = 0;
            for (auto &x : sp) if (!sk.count(x)) dif++;
            cout << "   -> " << dif << " aristas distintas (pesos repetidos: existen varios MST de igual costo)\n";
        }
    }
    return 0;
}
