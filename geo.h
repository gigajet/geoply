#ifndef _geo_h_
#define _geo_h_

#define SFML_STATIC
#include <vector>
#include <cmath>
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <limits>

struct Edge {
	bool isArc;
	//if isArc==true
	int xc, yc;
};
/*
  n points: (x[0],y[0]) to (x[n-1], y[n-1]), LISTED COUNTER CLOCKWISE.
  n edges: edge[i] connect point i to point i+1. (ORDER MATTERS, ex SEMICIRCLE)
  edge[n-1] connect point n-1 to point 0.
*/
struct Shape {
	std::vector<int> x;
	std::vector<int> y;
	std::vector<Edge> e;
};
int ccw (float x1, float y1, float x2, float y2, float x3, float y3) {
	const float eps=1e-2;
	float xAB = x2-x1, yAB=y2-y1,
		  xAC = x3-x1, yAC=y3-y1;
	double det = 1.0f*xAB*yAC - 1.0f*xAC*yAB;
	if (abs(det) < eps) return 0;
	if (det >= eps) return -1; //Ngược với thông thường, vì trục y quay xuống.
	return 1;
}
int ccw (int x1, int y1, int x2, int y2, int x3, int y3) {
	return ccw((float)x1, (float)y1, (float)x2, (float)y2, (float)x3, (float)y3);
}
int dsq_ (int x1, int y1, int x2, int y2) {
	return (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
}
float dsq_ (float x1, float y1, float x2, float y2) {
	return (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
}
double dot (double x1, double y1, double x2, double y2) {
	return x1*x2+y1*y2;
}
double veclen (double x, double y) {
	return sqrt(x*x+y*y);
}
double veclen (sf::Vector2f v) {
	return sqrt(v.x*v.x+v.y*v.y);
}

/*
  New coordinate if the axes rotates COUNTER-CLOCKWISE by an angle theta.
  FFS, theta is in DEGREE
*/
sf::Vector2f axestilt (float x, float y, float theta) {
	//std::cerr<<"Call axestilt(x="<<x<<",y="<<y<<",theta="<<theta<<")"<<std::endl;
	float rad = theta * acos(-1.0) / 180;
	//std::cerr<<"RAD: "<<rad<<std::endl;
	//Because y axis is downward, usual formula is now diffrent.
	float xp = x*cos(rad) - y*sin(rad);
	float yp = y*cos(rad) + x*sin(rad);
	//std::cerr<<"xp: "<<xp<<" yp: "<<yp<<std::endl<<std::endl;
	return sf::Vector2f(xp, yp);
}
sf::Vector2f axestilt (sf::Vector2f v, float theta) {
	return axestilt(v.x, v.y, theta);
}
/*
  Rotate a vector COUNTER-CLOCKWISE by an angle theta.
  FFS, theta is in DEGREE
*/
sf::Vector2f rotate (float x, float y, float theta) {
	//Vector tilt CCW by theta <-> Axes tilt CW by theta
	return axestilt(x, y, -theta);
}
sf::Vector2f rotate (sf::Vector2f v, float theta) {
	return rotate(v.x, v.y, theta);
}
/*
  Return [-180..180], counter-clockwise angle is POSITIVE
  FFS, angle return in DEGREE
*/
float angle (float x1, float y1, float x2, float y2) {
	float cos_theta = (x1*x2+y1*y2)/sqrt(x1*x1+y1*y1)/sqrt(x2*x2+y2*y2);
	float theta = acos(cos_theta);
	float deg = theta * 180 / acos(-1.0);
	//std::cerr<<"CCW: "<<ccw(0.,0.,x1,y1,x2,y2)<<std::endl;
	if (ccw(0.,0.,x1,y1,x2,y2)>=0)
		return deg;
	else return -deg;
}
float angle (sf::Vector2f v1, sf::Vector2f v2) {
	return angle (v1.x, v1.y, v2.x, v2.y);
}


void nearest_point_minimize (float x, float y, sf::Vector2f &ans, float &mindsq, float Mx, float My) {
	if (dsq_(x, y, Mx, My) < mindsq) {
		mindsq = dsq_(x, y, Mx, My);
		ans = {Mx, My};
	}
}

bool coincide (float x1, float y1, float x2, float y2, float kNearRadius=0.1) {
	return dsq_(x1,y1,x2,y2)<kNearRadius*kNearRadius;
}

/*
  Find point M belongs to Shape sh and nearest to (x,y).
  Distance function is the squared Euclid distance.
  Point and edge MUST BE LISTED COUNTER-CLOCKWISE. Otherwise, this function needs change.
*/
sf::Vector2f nearest_point (const Shape &sh, float x, float y) {
	int n = (int)sh.e.size();
	sf::Vector2f ans = {-1.0, -1.0};
	float mindsq = std::numeric_limits<float>::max();
	for (int i=0; i<n; ++i) {
		nearest_point_minimize(x,y,ans,mindsq,1.f*sh.x[i],1.f*sh.y[i]);
		if (sh.e[i].isArc) {
			//Special case: (x,y) coincide with center of circle. In that case, any point 
			//in the circumference will do.
			if (coincide(x,y,1.f*sh.e[i].xc, 1.f*sh.e[i].yc)) {
				nearest_point_minimize(x,y,ans,mindsq,1.f*sh.x[i],1.f*sh.y[i]);
				continue;
			}
			sf::Vector2f IA = {1.f*(sh.x[i]-sh.e[i].xc), 1.f*(sh.y[i]-sh.e[i].yc)};
			sf::Vector2f IP = {x-sh.e[i].xc, y-sh.e[i].yc};
			sf::Vector2f IB = {1.f*(sh.x[(i+1)%n]-sh.e[i].xc), 1.f*(sh.y[(i+1)%n]-sh.e[i].yc)};

			float theta = angle(IA, IP), theta0 = angle(IA, IB);

			/*
			if (i==0) {
				std::cerr<<"IA: "<<IA.x<<" "<<IA.y<<" | IB: "<<IB.x<<" "<<IB.y<<" | IP: "<<IP.x<<IP.y<<std::endl;
				std::cerr<<"Theta: "<<theta<<"\nTheta0: "<<theta0<<std::endl;
				std::cerr<<"ccw(IA,IP): "<<ccw(0.,0.,IA.x,IA.y,IP.x,IP.y)<<std::endl;
			}
			*/

			if (theta>=0 && theta<=theta0) {
				sf::Vector2f IQ = rotate(IA, theta);
				nearest_point_minimize(x,y,ans,mindsq,IQ.x+sh.e[i].xc,IQ.y+sh.e[i].yc);
			}
			else if (theta<0 && theta+180<=theta0) { //Trực giác nói rằng nhánh này không cần.
				sf::Vector2f IQ = rotate(IA, theta+180);
				nearest_point_minimize(x,y,ans,mindsq,IQ.x+sh.e[i].xc,IQ.y+sh.e[i].yc);
			}
		}
		else {
			//Độ chính xác không cao bằng tính comp và proj, nhưng ko chia nên ko xét ngoại lệ.
			float xA=1.f*sh.x[i], xB=1.f*sh.x[(i+1)%n], yA=1.f*sh.y[i], yB=1.f*sh.y[(i+1)%n];
			while (!coincide(xA,yA,xB,yB)) {
				float xM = (xA+xB)/2, yM=(yA+yB)/2;
				if (dsq_(xA, yA, x, y) < dsq_(xB, yB, x, y)) {
					xB = xM; yB = yM;
				}
				else {
					xA = xM; yA = yM;
				}
			}
			nearest_point_minimize(x,y,ans,mindsq,xA,yA);
			nearest_point_minimize(x,y,ans,mindsq,xB,yB); //dư, nhưng cho chắc.
		}
	} //for
	return ans;
}
#endif