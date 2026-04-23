#include <SFML/Graphics.hpp>
#include <math.h>
#include <algorithm>
#include "qua.hpp"
#include <iostream>
#include <cfloat>
#include <climits>
#include <vector>
#include <fstream>
#include <sstream>


//////////////////////////////////////////
int Width =     250;
int Height =    250;
int Scale =     4;
int FPS =       30;
double FOV =    M_PI*double(90)/180;
double Speed=   120;
double RotSpeed=120;
double Znear =  0.1;
double Zfar =   1000;
//////////////////////////////////////////




// WINDOW VARIABLES
sf::RenderWindow window(sf::VideoMode(Width*Scale, Height*Scale), "3D");
std::vector<std::vector<double> >  depthBuffer(Width, std::vector<double>(Height, Zfar));



// DRAW PIXEL
sf::RectangleShape tmp_px({Scale, Scale});
void drawPixel(int x, int y, sf::Color col){
    tmp_px.setPosition(x*Scale, y*Scale);
    tmp_px.setFillColor(col);
    window.draw(tmp_px);
}


// DRAW LINE
// void drawLine(const sf::Vector3<double> &p1, const sf::Vector3<double> &p2, sf::Color col){
//     if(p1.z < 1 || p2.z < 1){return;}
//     int x1 = p1.x+Width/2+0.5;
//     int y1 = -p1.y+Height/2+0.5;
//     int x2 = p2.x+Width/2+0.5;
//     int y2 = -p2.y+Height/2+0.5;
//     if(x1==x2 && y1==y2){drawPixel(x1,y1,col); return;}
//     if(abs(x2-x1)>abs(y2-y1)){
//         if(x2<x1){std::swap(x1,x2); std::swap(y1,y2);}
//         double inc = (double(y2)-y1)/(double(x2)-x1);
//         double nh = y1;
//         for(int i=x1; i<=x2; ++i){
//             drawPixel(i, nh+0.5, col);
//             nh+=inc;
//         }
//     }
//     else{
//         if(y2<y1){std::swap(x1,x2); std::swap(y1,y2);}
//         double inc = (double(x2)-x1)/(double(y2)-y1);
//         double nh = x1;
//         for(int i=y1; i<=y2; ++i){
//             drawPixel(nh+0.5, i, col);
//             nh+=inc;
//         }
//     }
// }


// PERSPECTIVE PROJECTION
const double ctg = double(1)/tan(FOV/2);
sf::Vector3<double> makePerspective(const sf::Vector3<double> &pt){
    sf::Vector3<double> ans = pt;

    if(ans.z==0){ans.x=DBL_MAX; ans.y=DBL_MAX; return ans;}

    ans.x = ans.x*Width*ctg/(2*ans.z);
    ans.y = ans.y*Height*ctg/(2*ans.z);

    return ans;
}

// DOT PRODUCT OF 2 VECTORS

double dotProduct(const sf::Vector3<double> &V1, const sf::Vector3<double> &V2){
    return (V1.x*V2.x + V1.y*V2.y + V1.z*V2.z);
}


// POLYGONS without perspective projection


struct Polygon{

    sf::Vector3<double> Node[3];
    sf::Vector3<double> Normal;
    sf::Color color = sf::Color(255,255,255, 255);

    Polygon(const sf::Vector3<double> &v1, const sf::Vector3<double> &v2, const sf::Vector3<double> &v3){
        Node[0]=v1;
        Node[1]=v2;
        Node[2]=v3;
        Normal = crossProduct();
    }

    Polygon(){;}

    sf::Vector3<double> crossProduct(){
        
        sf::Vector3<double> V1 = Node[1]-Node[0];
        sf::Vector3<double> V2 = Node[2]-Node[0];

        sf::Vector3<double> res = {V1.y*V2.z-V1.z*V2.y, V1.z*V2.x-V1.x*V2.z, V1.x*V2.y-V1.y*V2.x};
        double mg = sqrt(res.x*res.x + res.y*res.y + res.z*res.z);
        if(mg!=0){      
            res.x/=mg;
            res.y/=mg;
            res.z/=mg;
        }
        return res;

    }

    double getZ(double x, double y){
        return (-((x-Node[0].x)*Normal.x+(y-Node[0].y)*Normal.y))/Normal.z+Node[0].z;
    }

    

    void draw(){
        if(Node[0].z < Znear || Node[1].z < Znear || Node[2].z < Znear){return;}
        double dot = dotProduct(Normal, {0,0,-1});
        if(dot <= 0){return;}
        int mnx = std::min({Node[0].x, Node[1].x, Node[2].x}), mny = std::min({Node[0].y, Node[1].y, Node[2].y});
        int mxx = std::max({Node[0].x, Node[1].x, Node[2].x}), mxy = std::max({Node[0].y, Node[1].y, Node[2].y});
        int cl = dot;
        sf::Color col(color.r*dot, color.g*dot, color.b*dot, 255);

        for(int y = std::max(mny,-(Height>>1)); y<=std::min(mxy, (Height>>1)-1); ++y){
            for(int x = std::max(mnx,-(Width>>1)); x<=std::min(mxx, (Width>>1)-1); ++x){
                if((Node[1].x-Node[0].x)*(y-Node[0].y)-(Node[1].y-Node[0].y)*(x-Node[0].x) > 0){continue;}
                if((Node[2].x-Node[1].x)*(y-Node[1].y)-(Node[2].y-Node[1].y)*(x-Node[1].x) > 0){continue;}
                if((Node[0].x-Node[2].x)*(y-Node[2].y)-(Node[0].y-Node[2].y)*(x-Node[2].x) > 0){continue;}
                double akZ = getZ(x,y);
                if(akZ>Znear && akZ<depthBuffer[Width/2 + x][Height/2 - y]){drawPixel(Width/2 + x, Height/2 - y, col); depthBuffer[Width/2 + x][Height/2 - y] = akZ;}
                
            }
        }


    }

};








struct Mesh{

    std::vector<sf::Vector3<double> > Vertice;
    std::vector<sf::Vector3<int> > Polygons;

    std::vector<sf::Vector3<double> > tmpNode;

    static std::vector<std::string> stringDivide(std::string str, char divChar){
        std::vector<std::string> ans(1, "");
        for(auto i: str){
            if(i==divChar && ans.back()!=""){ans.push_back(""); continue;}
            ans.back()+= i;
        }
        return ans;
    }

    static double str2double(const std::string &str){
        std::stringstream ss; ss<<str;
        double ans; ss>>ans;
        return ans;
    }   

    static int getUntil(const std::string &str, char c){

        int id=0;
        for(int i=0; i<str.size() && str[i]!=c; ++i){
            id = id*10;
            id = id+(str[i]-'0');
        }

        return id;
    }

    Mesh(std::string filename, const sf::Vector3<double> &pos, double size){
        std::ifstream file(filename);

        while(!file.eof()){
            std::string str;
            getline(file, str);
            auto lnStr = stringDivide(str, ' ');

            if(lnStr[0]=="v"){
                Vertice.emplace_back(str2double(lnStr[1]), str2double(lnStr[2]), str2double(lnStr[3]));
                Vertice.back()*=size;
                Vertice.back()+=pos;
                continue;
            }

            if(lnStr[0]=="f"){
                int i1 = getUntil(lnStr[1],'/')-1;
                Polygons.emplace_back(i1, getUntil(lnStr[2],'/')-1, getUntil(lnStr[3],'/')-1);
                for(int i=4; i<lnStr.size(); ++i){
                    Polygons.emplace_back(i1, Polygons.back().z, getUntil(lnStr[i], '/')-1);
                }    
            }

        }
        tmpNode.resize(Vertice.size());

    }

    Mesh(){;}

    void draw(const sf::Vector3<double> &pos, const Quaternion<double> &rot){
        for(int i=0; i<Vertice.size(); ++i){
            tmpNode[i] = Vertice[i] - pos;
            tmpNode[i] = makeRotation(tmpNode[i], rot);
            tmpNode[i] = makePerspective(tmpNode[i]); 
        }
        for(auto [n1, n2, n3] : Polygons){
            Polygon(tmpNode[n1], tmpNode[n2], tmpNode[n3]).draw();
        }
    }


};






int main(){

    window.setFramerateLimit(FPS);
    double deltaTime = double(1)/FPS;

    Mesh car("car.obj", {0,0,50}, 25.0);

    Quaternion<double> rtx = qpolar(M_PI*RotSpeed*deltaTime/360, {1,0,0});
    Quaternion<double> rty = qpolar(M_PI*RotSpeed*deltaTime/360, {0,1,0});

    Quaternion<double> rot(1,0,0,0);

    sf::Vector3<double> pos = {0,0,-70};

    while(window.isOpen()){

        sf::Event event;
        while(window.pollEvent(event)){
            if(event.type == sf::Event::Closed){
                window.close();
            }
        }
        
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)){rot =  rot * rty;}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)){rot =  rot * qconj(rty);}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)){rot = rtx * rot;}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)){rot = qconj(rtx) * rot;}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){pos = pos + makeRotation({0,0,Speed*deltaTime}, qconj(rot));}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){pos = pos - makeRotation({0,0,Speed*deltaTime}, qconj(rot));}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){pos = pos - makeRotation({Speed*deltaTime, 0,0}, qconj(rot));}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){pos = pos + makeRotation({Speed*deltaTime, 0,0}, qconj(rot));}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp)){pos = pos + makeRotation({0,Speed*deltaTime,0}, qconj(rot));}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown)){pos = pos - makeRotation({0,Speed*deltaTime,0}, qconj(rot));}

        window.clear();

        car.draw(pos, rot);

        window.display();
        for(auto &xA : depthBuffer){
            for(auto &yA : xA){
                yA = Zfar;
            }
        }
    }


}
