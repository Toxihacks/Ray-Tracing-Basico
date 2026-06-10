#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include <iostream>
#include <cmath>
#include <limits>
#include <vector>
#include <string>
#include <iomanip> // Para alinear el texto en la consola

#ifndef PI
#define PI 3.14159265358979323846
#endif

class vector;

// 1. Clase Point
class point {
public:
    float x, y, z;
    point() { x = y = z = 0.0f; }
    point(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    point& operator =(const point& p){
        x = p.x; y = p.y; z = p.z;
        return *this;
    }

    void Clear() {
        x = y = z = 0.0f;
    }

    vector operator -(const point& p) const;
};

// 2. Clase vector con sobrecarga
class vector {
public:
    float x, y, z;
    vector() { x = y = z = 0.0f; }
    vector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    vector(const point& p) { x = p.x; y = p.y; z = p.z; }
    operator point() const { return point(x, y, z); }

    void normalize() {
        float mag = std::sqrt(x*x + y*y + z*z);
        if (mag > 0.00001f) {
            x /= mag; y /= mag; z /= mag;
        }
    }
    float modulo() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    float operator *(const vector& v) const {
        return (x * v.x) + (y * v.y) + (z * v.z);
    }

    vector operator *(float escalar) const {
        return vector(x * escalar, y * escalar, z * escalar);
    }

    vector operator /(float escalar) const {
        return vector(x / escalar, y / escalar, z / escalar);
    }

    vector cross(const vector& v) const {
        return vector(
            (y * v.z) - (z * v.y),
            (z * v.x) - (x * v.z),
            (x * v.y) - (y * v.x)
        );
    }
};

inline vector point::operator -(const point& p) const {
    return vector(x - p.x, y - p.y, z - p.z);
}

inline vector operator -(const vector& v1, const vector& v2) {
    return static_cast<point>(v1) - static_cast<point>(v2);
}

inline point operator +(const point& p, const vector& v) {
    return point(p.x + v.x, p.y + v.y, p.z + v.z);
}

class matPuntos {
public:
    point **p;
    int i, j;

    matPuntos() { p = nullptr; i = 0; j = 0; }
    ~matPuntos() { liberar(); }

    void inicializa(int x, int y) {
        liberar();
        i = x;
        j = y;
        p = new point*[i];
        for (int a = 0; a < i; a++) {
            p[a] = new point[j];
            for (int b = 0; b < j; b++) {
                p[a][b].Clear();
            }
        }
    }

    void liberar() {
        if (p != nullptr) {
            for (int a = 0; a < i; a++) {
                delete[] p[a];
            }
            delete[] p;
            p = nullptr;
        }
        i = j = 0;
    }
};

// 3. Clase Plane
class plane {
public:
    class triangle {
    public:
        point p0, p1, p2, bc;
        vector n;

        triangle() {}

        void updateGeom() {
            bc.x = (p0.x + p1.x + p2.x) / 3.0f;
            bc.y = (p0.y + p1.y + p2.y) / 3.0f;
            bc.z = (p0.z + p1.z + p2.z) / 3.0f;

            vector edge1 = p1 - p0;
            vector edge2 = p2 - p0;
            n = edge1.cross(edge2);
            n.normalize();
        }
    };

    point *p;
    int NP;
    triangle *t;
    int NT;
    vector n;

    plane() { p = nullptr; NP = 0; t = nullptr; NT = 0; }
    ~plane() { clearPlane(); }

    void newPoint(int num){
        point *tmp = new point[NP + num];
        for (int i = 0; i < NP; i++) tmp[i] = p[i];
        delete[] p;
        p = tmp;
        NP = NP + num;
    }

    void NewPoints(int N) {
        point *tp = new point[NP + N];
        for (int i = 0; i < NP; i++) {
            tp[i] = p[i];
        }
        for (int i = NP; i < NP + N; i++) {
            tp[i].Clear();
        }
        if (NP > 0) {
            delete[] p;
        }
        p = tp;
        NP += N;
    }

    void DeletePoint(int IP) {
        if (IP >= 0 && IP < NP) {
            point *tp = new point[NP - 1];
            int j = 0;
            for (int i = 0; i < NP; i++) {
                if (i != IP) {
                    tp[j] = p[i];
                    j++;
                }
            }
            delete[] p;
            p = tp;
            NP -= 1;
        }
    }

    void NewTriangle(int N) {
        triangle *tt = new triangle[NT + N];
        for (int i = 0; i < NT; i++) {
            tt[i] = t[i];
        }
        if (NT > 0) {
            delete[] t;
        }
        t = tt;
        NT += N;
    }

    void DeleteTriangle(int IT) {
        if (IT >= 0 && IT < NT) {
            triangle *tt = new triangle[NT - 1];
            int j = 0;
            for (int i = 0; i < NT; i++) {
                if (i != IT) {
                    tt[j] = t[i];
                    j++;
                }
            }
            delete[] t;
            t = tt;
            NT -= 1;
        }
    }

    void PointGenTriangle() {
        if (NP < 3) return;
        NewTriangle(NP - 2);
        int idx = 1;
        for (int T = 0; T < NT; T++) {
            idx--;
            t[T].p0 = p[idx];
            idx++;
            if (idx == NP) idx = 0;
            t[T].p1 = p[idx];
            idx++;
            if (idx == NP) idx = 0;
            t[T].p2 = p[idx];
            idx++;

            t[T].updateGeom();
        }
    }

    void moreTriangles(int nd) {
        if (NP < 4 || p == nullptr || nd <= 0) return;

        int numVertices = nd + 1;
        matPuntos matriz;
        matriz.inicializa(numVertices, numVertices);

        vector V1 = p[2] - p[1];
        float m1 = V1.modulo();
        float p1_paso = m1 / nd;

        vector V2 = p[1] - p[0];
        float m2 = V2.modulo();
        float p2_paso = m2 / nd;

        vector dirV1 = V1 * (1.0f / m1);
        vector dirV2 = V2 * (1.0f / m2);

        for (int i = 0; i <= nd; i++) {
            for (int j = 0; j <= nd; j++) {
                matriz.p[i][j] = p[0] + (dirV2 * (p2_paso * i)) + (dirV1 * (p1_paso * j));
            }
        }

        vector normal_guardada = n;
        clearPlane();
        n = normal_guardada;

        int totalPuntos = numVertices * numVertices;
        NewPoints(totalPuntos);

        int indicePunto = 0;
        for (int i = 0; i < numVertices; i++) {
            for (int j = 0; j < numVertices; j++) {
                p[indicePunto] = matriz.p[i][j];
                indicePunto++;
            }
        }

        int totalTriangulos = (nd * nd) * 2;
        NewTriangle(totalTriangulos);

        int tIdx = 0;
        for (int i = 0; i < nd; i++) {
            for (int j = 0; j < nd; j++) {
                int v0 = (i * numVertices) + j;
                int v1 = (i * numVertices) + (j + 1);
                int v2 = ((i + 1) * numVertices) + (j + 1);
                int v3 = ((i + 1) * numVertices) + j;

                t[tIdx].p0 = p[v0];
                t[tIdx].p1 = p[v1];
                t[tIdx].p2 = p[v2];
                t[tIdx].updateGeom();
                tIdx++;

                t[tIdx].p0 = p[v0];
                t[tIdx].p1 = p[v2];
                t[tIdx].p2 = p[v3];
                t[tIdx].updateGeom();
                tIdx++;
            }
        }
    }

    void clearPlane(){
        delete[] p; p = nullptr; NP = 0;
        delete[] t; t = nullptr; NT = 0;
        n = vector(0.0f, 0.0f, 0.0f);
    }
};

// 4. Clase Source
class source {
public:
    point p;
    vector *rays;
    int NR;
    float E;

    source() { rays = nullptr; NR = 0; E = 0.0f; }
    ~source() { delete[] rays; }

    void createRays(double NumberOfRays) {
        int A[30][3]= {{0,1,0}, {0,2,0}, {0,3,0}, {0,4,0}, {0,5,0},
            {1,6,0}, {2,6,0}, {2,7,0}, {3,7,0}, {3,8,0},
            {4,8,0}, {4,9,0}, {5,9,0}, {5,10,0},{1,10,0},
            {6,11,0},{7,11,0},{8,11,0},{9,11,0},{10,11,0},
            {1,2,0}, {2,3,0}, {3,4,0}, {4,5,0}, {5,1,0},
            {6,7,0}, {7,8,0}, {8,9,0}, {9,10,0},{10,6,0}
        };
        int T[20][3]= {{0,1,0},   {1,2,0},   {2,3,0},   {3,4,0},   {4,0,0},
            {5,6,-1},  {6,7,0},   {7,8,-1},  {8,9,0},   {9,10,-1},
            {10,11,0}, {11,12,-1},{12,13,0}, {13,14,-1},{14,5,0},
            {15,16,-1},{16,17,-1},{17,18,-1},{18,19,-1},{19,15,-1}
        };
        int i,j,k,n,m,RAY;
        double S,R,xB,yB,zB,xC,yC,zC,c[8];

        if(NR > 0)
            delete[] rays;

        n = int(floor(sqrt((NumberOfRays-2)/10)+0.5));
        if(n < 1) n = 1;

        NR = int(2+10*pow(n,2));
        rays = new vector[NR];

        S = 2/sqrt(5);
        R = (5-sqrt(5))/5;
        rays[0].x = 0;
        rays[0].y = 0;
        rays[0].z = 1;

        for(i=1; i<6; i++) {
            rays[i].x = S*cos((PI*i*72)/180);
            rays[i].y = S*sin((PI*i*72)/180);
            rays[i].z = 1-R;
            rays[i+5].x = S*cos((72*PI*i)/180+(36*PI)/180);
            rays[i+5].y = S*sin((72*PI*i)/180+(36*PI)/180);
            rays[i+5].z = R-1;
        }
        rays[11].x = 0;
        rays[11].y = 0;
        rays[11].z = -1;
        RAY = 12;

        for(j=0; j<30; j++) {
            A[j][2] = RAY;
            xB = rays[A[j][0]].x;
            yB = rays[A[j][0]].y;
            zB = rays[A[j][0]].z;
            xC = rays[A[j][1]].x;
            yC = rays[A[j][1]].y;
            zC = rays[A[j][1]].z;
            c[0] = pow(xC,2)*(pow(yB,2)+pow(zB,2))+pow(yC*zB-yB*zC,2)-2*xB*xC*(yB*yC+zB*zC)+pow(xB,2)*(pow(yC,2)+pow(zC,2));
            c[1] = acos(xB*xC+yB*yC+zB*zC);
            c[2] = -xC*(yB*yC+zB*zC)+xB*(pow(yC,2)+pow(zC,2));
            c[3] = xC*(pow(yB,2)+pow(zB,2))-xB*(yB*yC+zB*zC);
            c[4] = pow(xC,2)*yB-xB*xC*yC+zC*(-yC*zB+yB*zC);
            c[5] = -xB*xC*yB+pow(xB,2)*yC+zB*(yC*zB-yB*zC);
            c[6] = pow(xC,2)*zB-xB*xC*zC+yC*(yC*zB-yB*zC);
            c[7] = -xB*xC*zB+pow(xB,2)*zC+yB*(-yC*zB+yB*zC);
            for(i=1; i<n; i++) {
                rays[RAY].x = (c[2]*cos(i*c[1]/n)+c[3]*cos((n-i)*c[1]/n))/c[0];
                rays[RAY].y = (c[4]*cos(i*c[1]/n)+c[5]*cos((n-i)*c[1]/n))/c[0];
                rays[RAY].z = (c[6]*cos(i*c[1]/n)+c[7]*cos((n-i)*c[1]/n))/c[0];
                RAY++;
            }
        }

        for(k=0; k<20; k++)
            for(j=1; j<n; j++) {
                xB = rays[A[T[k][0]][2]+j-1].x;
                yB = rays[A[T[k][0]][2]+j-1].y;
                zB = rays[A[T[k][0]][2]+j-1].z;
                xC = rays[A[T[k][1]][2]+j-1].x;
                yC = rays[A[T[k][1]][2]+j-1].y;
                zC = rays[A[T[k][1]][2]+j-1].z;
                c[0] = pow(xC,2)*(pow(yB,2)+pow(zB,2))+pow(yC*zB-yB*zC,2)-2*xB*xC*(yB*yC+zB*zC)+pow(xB,2)*(pow(yC,2)+pow(zC,2));
                c[1] = acos(xB*xC+yB*yC+zB*zC);
                c[2] = -xC*(yB*yC+zB*zC)+xB*(pow(yC,2)+pow(zC,2));
                c[3] = xC*(pow(yB,2)+pow(zB,2))-xB*(yB*yC+zB*zC);
                c[4] = pow(xC,2)*yB-xB*xC*yC+zC*(-yC*zB+yB*zC);
                c[5] = -xB*xC*yB+pow(xB,2)*yC+zB*(yC*zB-yB*zC);
                c[6] = pow(xC,2)*zB-xB*xC*zC+yC*(yC*zB-yB*zC);
                c[7] = -xB*xC*zB+pow(xB,2)*zC+yB*(-yC*zB+yB*zC);
                if(T[k][2]==0)m=j;
                else m=n-j;
                for(i=1; i<m; i++) {
                    rays[RAY].x = (c[2]*cos(i*c[1]/m)+c[3]*cos((m-i)*c[1]/m))/c[0];
                    rays[RAY].y = (c[4]*cos(i*c[1]/m)+c[5]*cos((m-i)*c[1]/m))/c[0];
                    rays[RAY].z = (c[6]*cos(i*c[1]/m)+c[7]*cos((m-i)*c[1]/m))/c[0];
                    RAY++;
                }
            }
    }
};

// NUEVA CLASE: MATRICES DE PRECÁLCULO (Patch-to-Patch)
class Matrices {
public:
    double **distancias;
    int **tiemposVuelo;
    double **angulosSolidos;
    int size;

    Matrices() : distancias(nullptr), tiemposVuelo(nullptr), angulosSolidos(nullptr), size(0) {}
    ~Matrices() { liberar(); }

    void inicializar(int n) {
        liberar();
        size = n;
        distancias = new double*[n];
        tiemposVuelo = new int*[n];
        angulosSolidos = new double*[n];
        for (int i = 0; i < n; i++) {
            distancias[i] = new double[n]();
            tiemposVuelo[i] = new int[n]();
            angulosSolidos[i] = new double[n]();
        }
    }

    void liberar() {
        if (distancias) {
            for (int i = 0; i < size; i++) {
                delete[] distancias[i];
                delete[] tiemposVuelo[i];
                delete[] angulosSolidos[i];
            }
            delete[] distancias;
            delete[] tiemposVuelo;
            delete[] angulosSolidos;
        }
        size = 0;
    }

    void calcular(plane& plano) {
        if (plano.NT <= 0) return;
        inicializar(plano.NT);

        std::vector<double> areasReales(size, 0.0);
        for (int i = 0; i < size; i++) {
            vector a1 = plano.t[i].p1 - plano.t[i].p0;
            vector a2 = plano.t[i].p2 - plano.t[i].p0;
            areasReales[i] = 0.5 * (a1.cross(a2)).modulo();
        }

        for (int i = 0; i < size; i++) {
            double sumaProyectada = 0.0;
            for (int j = 0; j < size; j++) {
                if (i == j) {
                    distancias[i][j] = 0.0;
                    tiemposVuelo[i][j] = 0;
                    angulosSolidos[i][j] = 0.0;
                } else {
                    vector vd = plano.t[j].bc - plano.t[i].bc;
                    distancias[i][j] = vd.modulo();
                    tiemposVuelo[i][j] = (int)((distancias[i][j] / 340.0) * 1000.0);

                    double areaProy = areasReales[j] * std::pow(0.2 / distancias[i][j], 2);
                    angulosSolidos[i][j] = areaProy;
                    sumaProyectada += areaProy;
                }
            }

            for (int j = 0; j < size; j++) {
                if (i != j && sumaProyectada > 0.0) {
                    angulosSolidos[i][j] = (angulosSolidos[i][j] / sumaProyectada) * 100.0;
                }
            }
        }
    }

    void imprimirReporte() {
        std::cout << "\n=== MATRIZ DE DISTANCIAS ===" << std::endl;
        for(int i = 0; i < size; i++) {
            for(int j = 0; j < size; j++) {
                std::cout << std::fixed << std::setprecision(2) << std::setw(8) << distancias[i][j];
            }
            std::cout << std::endl;
        }

        std::cout << "\n=== MATRIZ DE TIEMPOS DE VUELO (ms) ===" << std::endl;
        for(int i = 0; i < size; i++) {
            for(int j = 0; j < size; j++) {
                std::cout << std::setw(8) << tiemposVuelo[i][j];
            }
            std::cout << std::endl;
        }

        std::cout << "\n=== MATRIZ DE PORCENTAJES DE ANGULOS SOLIDOS (%) ===" << std::endl;
        for(int i = 0; i < size; i++) {
            for(int j = 0; j < size; j++) {
                std::cout << std::fixed << std::setprecision(2) << std::setw(8) << angulosSolidos[i][j];
            }
            std::cout << std::endl;
        }
    }
};

// 5. Clase Receptor
class receptor {
public:
    point p;
    float *Ei;
    int nIs;
    float ReceptionRadius;
    int timSim = 1000;

    receptor() {
        Ei = new float[timSim]();
        nIs = 0;
        ReceptionRadius = 0.5f;
    }
    ~receptor() { delete[] Ei; }

    void reception(point Po, vector Vi, float distAcumuladaPrev, float E) {
        vector vd, n, r;
        float d_proy;
        point pi;

        vd = p - Po;
        n = Vi / Vi.modulo() * (-1.0f);
        d_proy = (n * vd) / (n * Vi);
        pi = Po + (Vi * d_proy);
        r = p - pi;
        float d = r.modulo();

        if (ReceptionRadius >= d && d_proy >= 0.0f) {
            nIs++;
            float distTotalImpacto = distAcumuladaPrev + d_proy;
            int t_ms = (int)((distTotalImpacto / 340.0f) * 1000.0f);

            if (t_ms >= 0 && t_ms < timSim) {
                Ei[t_ms] += E;
            }
        }
    }
};

// 6. Clase room con animaciones implementadas
class room {
public:
    plane *P;
    int NP;
    source *S;
    int NS;
    receptor *R;
    int NR;

    std::vector<std::vector<point>> trayectorias;

    room() { P = nullptr; S = nullptr; R = nullptr; NP = 0; NS = 0; NR = 0; }
    ~room() { delete[] P; delete[] S; delete[] R; }

    void rayTracing() {
        trayectorias.clear();
        if (NS <= 0 || S == nullptr || NP <= 0 || P == nullptr) return;

        for (int r = 0; r < S[0].NR; r++) {
            std::vector<point> rayoActual;

            point origen_actual = S[0].p;
            vector direccion_actual = S[0].rays[r];
            direccion_actual.normalize();

            rayoActual.push_back(origen_actual);

            int ultimoPlanoIntersecado = -1;
            int conteoReflexiones = 0;
            bool rayoPerdido = false;

            float energia = S[0].E;
            float distanciaTotalRecorrida = 0.0f;

            while (conteoReflexiones < 50 && !rayoPerdido && energia > 0.01f) {
                float distanciaMinima = std::numeric_limits<float>::max();
                int planoIntersecado = -1;
                point puntoImpacto;

                for (int i = 0; i < NP; i++) {
                    vector N = P[i].n;
                    N.normalize();

                    float denominador = N * direccion_actual;

                    if (denominador < 0.0f && i != ultimoPlanoIntersecado) {
                        if (P[i].NP > 0 && P[i].p != nullptr) {
                            point vertice_plano = P[i].p[0];
                            vector Vd = vertice_plano - origen_actual;

                            float numerador = N * Vd;
                            float d_test = numerador / denominador;

                            if (d_test > 0.001f && d_test < distanciaMinima) {
                                distanciaMinima = d_test;
                                planoIntersecado = i;
                                puntoImpacto = origen_actual + (direccion_actual * d_test);
                            }
                        }
                    }
                }

                if (planoIntersecado != -1) {
                    conteoReflexiones++;

                    if (this->NR > 0 && this->R != nullptr) {
                        this->R[0].reception(origen_actual, direccion_actual, distanciaTotalRecorrida, energia);
                    }

                    distanciaTotalRecorrida += distanciaMinima;
                    energia *= (1.0f - 0.2f);

                    rayoActual.push_back(puntoImpacto);

                    vector N = P[planoIntersecado].n;
                    N.normalize();
                    float dotProduct = direccion_actual * N;

                    vector vr = direccion_actual - (N * (2.0f * dotProduct));
                    vr.normalize();

                    origen_actual = puntoImpacto;
                    direccion_actual = vr;
                    ultimoPlanoIntersecado = planoIntersecado;
                }
                else {
                    rayoPerdido = true;
                }
            }
            trayectorias.push_back(rayoActual);
        }
    }
};

int main() {
    sf::RenderWindow window(
        sf::VideoMode({800, 600}),
        "Visualizador RayTracing Animado - 12 Rayos con Fisica",
        sf::State::Windowed
    );
    window.setFramerateLimit(60);

    sf::RenderWindow windowMalla(
        sf::VideoMode({600, 600}),
        "Verificacion Malla (Triangulos del Plano Frontal)",
        sf::State::Windowed
    );
    windowMalla.setFramerateLimit(60);

    room r;
    r.NS = 1; r.S = new source[1];
    r.S[0].p = point(-1.0f, -1.0f, 1.0f); // Fuente
    r.S[0].E = 10.0f; // Asignación de la energía inicial requerida

    // Generador implementado desde la clase source
    r.S[0].createRays(12);

    r.NR = 1; r.R = new receptor[1];
    r.R[0].p = point(-1.0f, -1.0f, -1.0f); // Receptor

    r.NP = 6; r.P = new plane[6];
    r.P[0].newPoint(1); r.P[0].p[0] = point(2, 0, 0);   r.P[0].n = vector(-1, 0, 0);
    r.P[1].newPoint(1); r.P[1].p[0] = point(-2, 0, 0);  r.P[1].n = vector(1, 0, 0);
    r.P[2].newPoint(1); r.P[2].p[0] = point(0, 2, 0);   r.P[2].n = vector(0, -1, 0);
    r.P[3].newPoint(1); r.P[3].p[0] = point(0, -2, 0);  r.P[3].n = vector(0, 1, 0);

    r.P[4].NewPoints(4);
    r.P[4].p[0] = point(-2, -2, 2);
    r.P[4].p[1] = point(2, -2, 2);
    r.P[4].p[2] = point(2, 2, 2);
    r.P[4].p[3] = point(-2, 2, 2);
    r.P[4].n = vector(0, 0, -1);
    r.P[4].moreTriangles(4);

    r.P[5].newPoint(1); r.P[5].p[0] = point(0, 0, -2);  r.P[5].n = vector(0, 0, 1);

    r.rayTracing();

    // ==========================================
    // IMPRESIÓN DE ECHOGRAMA (Ei[t]) Y MATRICES
    // ==========================================
    std::cout << "\n=== REPORTE DE ECHOGRAMA (Energia vs Tiempo ms) ===" << std::endl;
    std::cout << "Impactos Registrados (nIs): " << r.R[0].nIs << "\n" << std::endl;

    for (int t = 0; t < r.R[0].timSim; t++) {
        if (r.R[0].Ei[t] > 0.0f) {
            std::cout << "T[" << t << " ms]: Energia = " << r.R[0].Ei[t] << std::endl;
        }
    }

    Matrices preCalculo;
    preCalculo.calcular(r.P[4]);
    preCalculo.imprimirReporte();


    window.setActive(true);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = 800.0f / 600.0f;
    gluPerspective(45.0f, aspect, 1.0f, 200.0f);

    float rotacionX = 20.0f;
    float rotacionY = -45.0f;
    float zoom = -10.0f;

    bool mousePresionado = false;
    sf::Vector2i posicionAnteriorMouse;

    float tiempoAcusticoReloj = 0.0f;
    float escalaVisual = 0.0003f;

    while (window.isOpen() || windowMalla.isOpen()) {

        tiempoAcusticoReloj += escalaVisual;
        float distanciaFisicaRayo = 340.0f * tiempoAcusticoReloj;

        if (distanciaFisicaRayo > 40.0f) tiempoAcusticoReloj = 0.0f;

        if (window.isOpen()) {
            while (const std::optional<sf::Event> event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) window.close();

                if (const auto* mouseButtonEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseButtonEvent->button == sf::Mouse::Button::Left) {
                        mousePresionado = true;
                        posicionAnteriorMouse = sf::Mouse::getPosition(window);
                    }
                }

                if (const auto* mouseButtonEvent = event->getIf<sf::Event::MouseButtonReleased>()) {
                    if (mouseButtonEvent->button == sf::Mouse::Button::Left) {
                        mousePresionado = false;
                    }
                }

                if (event->is<sf::Event::MouseMoved>() && mousePresionado) {
                    sf::Vector2i posicionActualMouse = sf::Mouse::getPosition(window);
                    float deltaX = static_cast<float>(posicionActualMouse.x - posicionAnteriorMouse.x);
                    float deltaY = static_cast<float>(posicionActualMouse.y - posicionAnteriorMouse.y);
                    rotacionY += deltaX * 0.5f;
                    rotacionX += deltaY * 0.5f;
                    posicionAnteriorMouse = posicionActualMouse;
                }

                if (const auto* mouseWheelEvent = event->getIf<sf::Event::MouseWheelScrolled>()) {
                    if (mouseWheelEvent->wheel == sf::Mouse::Wheel::Vertical) {
                        zoom += mouseWheelEvent->delta * 2.0f;
                    }
                }
            }
        }

        if (windowMalla.isOpen()) {
            while (const std::optional<sf::Event> event = windowMalla.pollEvent()) {
                if (event->is<sf::Event::Closed>()) windowMalla.close();
            }
        }

        if (window.isOpen()) {
            window.setActive(true);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glTranslatef(0.0f, 0.0f, zoom);
            glRotatef(rotacionX, 1.0f, 0.0f, 0.0f);
            glRotatef(rotacionY, 0.0f, 1.0f, 0.0f);

            glPointSize(10.0f);
            glBegin(GL_POINTS);
                glColor3f(1.0f, 1.0f, 0.0f);
                glVertex3f(r.S[0].p.x, r.S[0].p.y, r.S[0].p.z);

                glColor3f(0.0f, 1.0f, 0.0f);
                glVertex3f(r.R[0].p.x, r.R[0].p.y, r.R[0].p.z);
            glEnd();

            glColor3f(0.3f, 0.3f, 0.4f);
            glLineWidth(1.0f);
            glBegin(GL_LINES);
                glVertex3f(-2, -2, -2); glVertex3f(2, -2, -2);
                glVertex3f(2, -2, -2);  glVertex3f(2, 2, -2);
                glVertex3f(2, 2, -2);   glVertex3f(-2, 2, -2);
                glVertex3f(-2, 2, -2);  glVertex3f(-2, -2, -2);

                glVertex3f(-2, -2, 2);  glVertex3f(2, -2, 2);
                glVertex3f(2, -2, 2);   glVertex3f(2, 2, 2);
                glVertex3f(2, 2, 2);    glVertex3f(-2, 2, 2);
                glVertex3f(-2, 2, 2);   glVertex3f(-2, -2, 2);

                glVertex3f(-2, -2, -2); glVertex3f(-2, -2, 2);
                glVertex3f(2, -2, -2);  glVertex3f(2, -2, 2);
                glVertex3f(2, 2, -2);   glVertex3f(2, 2, 2);
                glVertex3f(-2, 2, -2);  glVertex3f(-2, 2, 2);
            glEnd();

            for (size_t k = 0; k < r.trayectorias.size(); ++k) {
                if (r.trayectorias[k].empty()) continue;

                int puntos_totales = r.trayectorias[k].size();

                glColor3f(0.0f, 1.0f, 1.0f);
                glLineWidth(3.0f);
                glBegin(GL_LINE_STRIP);

                float d_acum = 0.0f;
                int idx_dibujado = 0;

                for (int i = 0; i < puntos_totales - 1; i++) {
                    vector v_tramo = r.trayectorias[k][i+1] - r.trayectorias[k][i];
                    float d_tramo = v_tramo.modulo();

                    if (d_acum + d_tramo > distanciaFisicaRayo) {
                        float t_proy = (distanciaFisicaRayo - d_acum) / d_tramo;
                        point puntaActual = r.trayectorias[k][i] + (v_tramo * t_proy);
                        glVertex3f(r.trayectorias[k][i].x, r.trayectorias[k][i].y, r.trayectorias[k][i].z);
                        glVertex3f(puntaActual.x, puntaActual.y, puntaActual.z);
                        idx_dibujado = i;
                        break;
                    } else {
                        glVertex3f(r.trayectorias[k][i].x, r.trayectorias[k][i].y, r.trayectorias[k][i].z);
                        d_acum += d_tramo;
                        idx_dibujado = i + 1;
                    }
                }

                if (d_acum >= distanciaFisicaRayo && idx_dibujado == puntos_totales - 1) {
                    glVertex3f(r.trayectorias[k].back().x, r.trayectorias[k].back().y, r.trayectorias[k].back().z);
                }
                glEnd();

                glColor3f(1.0f, 0.0f, 0.0f);
                glPointSize(5.0f);
                glBegin(GL_POINTS);
                for(int i = 1; i <= idx_dibujado; i++) {
                    glVertex3f(r.trayectorias[k][i].x, r.trayectorias[k][i].y, r.trayectorias[k][i].z);
                }
                glEnd();
            }

            window.display();
        }

        if (windowMalla.isOpen()) {
            windowMalla.setActive(true);

            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(-3.0, 3.0, -3.0, 3.0, -1.0, 1.0);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            for (int i = 0; i < r.P[4].NT; i++) {
                glColor3f(0.0f, 0.8f, 0.6f);
                glLineWidth(1.5f);
                glBegin(GL_LINE_LOOP);
                    glVertex2f(r.P[4].t[i].p0.x, r.P[4].t[i].p0.y);
                    glVertex2f(r.P[4].t[i].p1.x, r.P[4].t[i].p1.y);
                    glVertex2f(r.P[4].t[i].p2.x, r.P[4].t[i].p2.y);
                glEnd();

                glColor3f(1.0f, 0.2f, 0.2f);
                glPointSize(4.0f);
                glBegin(GL_POINTS);
                    glVertex2f(r.P[4].t[i].bc.x, r.P[4].t[i].bc.y);
                glEnd();
            }

            windowMalla.display();
        }
    }

    return 0;
}
