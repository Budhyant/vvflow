#ifndef ELEMENTARY_H_
#define ELEMENTARY_H_

#include <iostream>

const double C_PI =		3.14159265358979323846; 	// = PI
const double C_2PI =	6.28318530717958647692; 	// = 2*PI
const double C_1_2PI =	0.15915494309189533577;		// = 1/(2*PI)
const double C_1_PI = 	0.31830988618379067154; 	// = 1/PI
const double C_2_PI = 	0.63661977236758134308; 	// = 2/PI


/******************* Vectors *******************/

struct vector
{
	double x;
	double y;
};
typedef struct vector Vector;
typedef struct vector Point;

#define ZeroPoint(v) (v).x=0; (v).y=0
#define InitPoint(v, sx, sy) (v).x=(sx); (v).y=(sy)

double abs(Vector v);
double abs2(Vector v);

double operator * (const Vector &v1, const Vector &v2);
double operator & (const Vector &v1, const Vector &v2); //Vector multiplication

Point operator + (const Point &p1, const Point &p2);
Point operator - (const Point &p1, const Point &p2);

void operator +=(Point &p1,const Point &p2);
void operator -=(Point &p1,const Point &p2);
void operator *=(Point &p,const double a);
void operator /=(Point &p,const double a);

Point operator - (const Point &p);
Point operator + (const Point &p);
Point operator * (const Point &p, const double &a);
Point operator * (const double& a, const Point &p);
Point operator / (const Point &p, const double &a);

std::ostream& operator<< (std::ostream& os, const Point &p);

Point rotl(const Point &p);
Point rotr(const Point &p);

/********************* Vortex ********************/

struct vortex
{
	double rx, ry;
	double vx, vy;
	//double vxtmp, vytmp;
	double g;
	//char flag;
	// flags:
	//	1 - to be removed;
	//	2 - to be merged;
};
typedef struct vortex TVortex;
typedef struct vortex TObject;

inline void InitObject(TObject &v, double x, double y, double g)
{ v.rx = x; v.ry = y; v.g = g; v.vx = v.vy = 0; }

inline void ZeroObject(TObject &v)
{ v.rx = v.ry = v.g = v.vx = v.vy = 0; } 

#define InitVortex(v, sx, sy, sg) v.rx=sx; v.ry=sy; v.vx=v.vy=0; v.g=sg; //v.flag=0;
#define ZeroVortex(v) InitVortex(v, 0, 0, 0);

std::ostream& operator<< (std::ostream& os, const TVortex &v);

#endif
