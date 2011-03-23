#include "body.h"
#include "stdio.h"
#include "iostream"
#include "math.h"
using namespace std;

TBody::TBody(double (*sRotationV)(double Time),
				double sRotationAxisX, double sRotationAxisY)
{
	List = new TList<TObject>();
	RotationV = sRotationV;
	RotationAxisX = sRotationAxisX;
	RotationAxisY = sRotationAxisY;
}

int TBody::LoadFromFile(const char* filename)
{
	if (!this) return -1;
	if ( !List ) return -1;
	FILE *fin;

	fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called " << filename << endl; return -1; } 

	TObject Obj; ZeroObject(Obj);
	char line[255];
	while ( fgets(line, 254, fin) )
	{
		if (sscanf(line, "%lf\t%lf\n", &Obj.rx, &Obj.ry) == 2)
			List->Copy(&Obj);
	}
	fclose(fin);

	return 0;
}

void TBody::Rotate(double dt)
{
	if (!this) return;
	double alpha = RotationVVar*dt;

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for (; Obj<LastObj; Obj++)
	{
		Obj->rx+= (Obj->ry - RotationAxisY)*alpha;
		Obj->ry-= (Obj->rx - RotationAxisX)*alpha;
	}
}

bool TBody::PointIsValid(double x, double y)
{
	if (!this) return true;

	bool res = true;

	TObject *i = List->First;
	TObject *j = List->Last;
	TObject *&LastVort = List->Last;
	for ( ; i<LastVort; j=i++)
	{
		if ((
			(i->ry < j->ry) && (i->ry <= y) && (y <= j->ry) &&
			((j->ry - i->ry) * (x - i->rx) > (j->rx - i->rx) * (y - i->ry))
			) || (
			(i->ry > j->ry) && (j->ry <= y) && (y <= i->ry) &&
			((j->ry - i->ry) * (x - i->rx) < (j->rx - i->rx) * (y - i->ry))
		)) res = !res;
	}

	return res;
}

double TBody::SurfaceLenght()
{
	if (!this) return 0;
	double res=0;

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for (; Obj<LastObj; Obj++)
	{
		double dx = (Obj->rx - (Obj+1)->rx);
		double dy = (Obj->ry - (Obj+1)->ry);
		res += sqrt(dx*dx+dy*dy);
	}

	return res;
}

/************************** HEAT LAYER ****************************************/

void TBody::CleanHeatLayer()
{
	if (!this) return;
	if (!HeatLayer) return;

	for (int i=0; i<List->size; i++)
	{
		HeatLayer[i]=0;
	}
}

int* TBody::ObjectIsInHeatLayer(TObject &obj)
{
	return false;
}
