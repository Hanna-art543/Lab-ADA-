#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>
using namespace std;

using Arista = tuple<int, string, string>;
vector<Arista> aristas = {
    {4, "Base", "Refugio"}, {2, "Base", "Veterinaria"},
    {7, "Base", "Tienda"}, {3, "Refugio", "Veterinaria"},
    {5, "Refugio", "Adopciones"}, {6, "Veterinaria", "Tienda"},
    {1, "Veterinaria", "Adopciones"}, {8, "Tienda", "Adopciones"}
};

vector<Arista> prim() {
    set<string> visitados = {"Base"};
    vector<Arista> resultado;
    while (visitados.size() < 5) {
        Arista mejor = {1000000, "", ""};
        for (auto e : aristas) {
            auto [p, a, b] = e;
            if (visitados.count(a) != visitados.count(b) && e < mejor)
                mejor = e;
        }
        auto [p, a, b] = mejor;
        visitados.insert(a);
        visitados.insert(b);
        resultado.push_back(mejor);
    }
    return resultado;
}

vector<Arista> kruskal() {
    map<string, string> padre;
    for (auto [p, a, b] : aristas) padre[a] = a, padre[b] = b;
    auto raiz = [&](string x) {
        while (padre[x] != x) x = padre[x];
        return x;
    };
    auto ordenadas = aristas;
    sort(ordenadas.begin(), ordenadas.end());
    vector<Arista> resultado;
    for (auto [p, a, b] : ordenadas) {
        string x = raiz(a), y = raiz(b);
        if (x != y) {
            padre[x] = y;
            resultado.push_back({p, a, b});
        }
    }
    return resultado;
}

void mostrar(string nombre, vector<Arista> resultado) {
    int costo = 0;
    cout << "\n" << nombre << ":\n";
    for (auto [p, a, b] : resultado) {
        cout << a << " - " << b << ": " << p << "\n";
        costo += p;
    }
    cout << "Costo minimo: " << costo << "\n";
}

int main() {
    mostrar("Prim", prim());
    mostrar("Kruskal", kruskal());
}
