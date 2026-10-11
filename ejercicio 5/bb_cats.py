aristas = [
    (4, "Base", "Refugio"),
    (2, "Base", "Veterinaria"),
    (7, "Base", "Tienda"),
    (3, "Refugio", "Veterinaria"),
    (5, "Refugio", "Adopciones"),
    (6, "Veterinaria", "Tienda"),
    (1, "Veterinaria", "Adopciones"),
    (8, "Tienda", "Adopciones")
]

def prim():
    visitados = {"Base"}
    resultado = []
    while len(visitados) < 5:
        p, a, b = min((p, a, b) for p, a, b in aristas if (a in visitados) != (b in visitados))
        visitados.update([a, b])
        resultado.append((p, a, b))
    return resultado

def kruskal():
    padre = {x: x for _, a, b in aristas for x in (a, b)}
    def raiz(x):
        while padre[x] != x:
            x = padre[x]
        return x

    resultado = []
    for p, a, b in sorted(aristas):
        x, y = raiz(a), raiz(b)
        if x != y:
            padre[x] = y
            resultado.append((p, a, b))
    return resultado

for nombre, algoritmo in [("Prim", prim), ("Kruskal", kruskal)]:
    resultado = algoritmo()
    print(f"\n{nombre}:")
    for p, a, b in resultado:
        print(f"{a} - {b}: {p}")
    print("Costo minimo:", sum(p for p, _, _ in resultado))
