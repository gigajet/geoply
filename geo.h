#ifndef _geo_h_
#define _geo_h_

#define SFML_STATIC
#include <vector>
#include <cmath>
#include <SFML/System/Vector2.hpp>
#include <iostream>

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
	if (det >= eps) return 1;
	return -1;
}
int ccw (int x1, int y1, int x2, int y2, int x3, int y3) {
	return ccw((float)x1, (float)y1, (float)x2, (float)y2, (float)x3, (float)y3);
}
int dsq_ (int x1, int y1, int x2, int y2) {
	return (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
}
double dot (double x1, double y1, double x2, double y2) {
	return x1*x2+y1*y2;
}
double veclen (double x, double y) {
	return sqrt(x*x+y*y);
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
	if (ccw(0.,0.,x1,y1,x2,y2)>=0)
		return deg;
	else return -deg;
}
float angle (sf::Vector2f v1, sf::Vector2f v2) {
	return angle (v1.x, v1.y, v2.x, v2.y);
}
#endif