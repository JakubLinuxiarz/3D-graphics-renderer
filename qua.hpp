#include <SFML/Graphics.hpp>
#include <math.h>

template <typename T>
struct Quaternion{
    T r = 0;
    T i = 0;
    T j = 0;
    T k = 0;

    Quaternion<T> (){;}
    Quaternion<T> (T r, T i, T j, T k): r(r), i(i), j(j), k(k){}
    Quaternion<T> (const sf::Vector3<T> &vec): r(0), i(vec.x), j(vec.y), k(vec.z){}
};


// OSTREAM
template<typename T>
std::ostream & operator<< (std::ostream &out, const Quaternion<T> &qua){
    out<<"( "<<qua.r<<" + "<<qua.i<<"i + "<<qua.j<<"j + "<<qua.k<<"k)";
    return out;
}

// ADD
template <typename T>
Quaternion<T> operator+ (const Quaternion<T> &q1, const Quaternion<T> &q2){
    Quaternion<T> res;
    res.r = q1.r + q2.r;
    res.i = q1.i + q2.i;
    res.j = q1.j + q2.j;
    res.k = q1.k + q2.k;
    return res;
}

// SUBSTRUCT
template <typename T>
Quaternion<T> operator- (const Quaternion<T> &q1, const Quaternion<T> &q2){
    Quaternion<T> res;
    res.r = q1.r - q2.r;
    res.i = q1.i - q2.i;
    res.j = q1.j - q2.j;
    res.k = q1.k - q2.k;
    return res;
}

// MULTIPLY
template <typename T>
Quaternion<T> operator* (const Quaternion<T> &q1, const Quaternion<T> &q2){
    Quaternion<T> res;
    res.r = q1.r*q2.r - q1.i*q2.i - q1.j*q2.j - q1.k*q2.k;
    res.i = q1.i*q2.r + q1.r*q2.i + q1.j*q2.k - q1.k*q2.j;
    res.j = q1.j*q2.r + q1.r*q2.j + q1.k*q2.i - q1.i*q2.k;
    res.k = q1.k*q2.r + q1.r*q2.k + q1.i*q2.j - q1.j*q2.i;
    return res;
}

// CONJUGATE
template <typename T>
Quaternion<T> qconj(const Quaternion<T> &q){
    Quaternion<T> res;
    res.r = q.r;
    res.i = -q.i;
    res.j = -q.j;
    res.k = -q.k;
    return res;
}

// POLAR
template<typename T>
Quaternion<T> qpolar(T alpha, T x, T y, T z){
    T qcos = cos(alpha);
    T qsin = sin(alpha);

    Quaternion<T> res(qcos, qsin*x, qsin*y, qsin*z);
    return res;
}

template<typename T>
Quaternion<T> qpolar(T alpha, const sf::Vector3<T> &vec){
    T qcos = cos(alpha);
    T qsin = sin(alpha);

    Quaternion<T> res(qcos, qsin*vec.x, qsin*vec.y, qsin*vec.z);
    return res;
}


// MAGNITUDE
template<typename T>
T qmagnitude(const Quaternion<T> &q){
    return sqrt(q.r*q.r + q.i*q.i + q.j*q.j + q.k*q.k);
}


// VECTOR NORMALIZE
template<typename T>
sf::Vector3<T> normalize(const sf::Vector3<T> &vec){

    double mg = sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);

    sf::Vector3<T> res = {vec.x/mg, vec.y/mg, vec.z/mg};
    
    return res;
}

// POINT ROTATION
template<typename T>
sf::Vector3<T> makeRotation(const sf::Vector3<T> &pt, const Quaternion<T> &rot){
    Quaternion<T> res = rot*Quaternion(pt)*qconj(rot);
    sf::Vector3<T> ans = {res.i, res.j, res.k};
    return ans;
}
