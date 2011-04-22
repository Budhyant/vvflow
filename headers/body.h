#ifndef BODY_H_
#define BODY_H_

#include "elementary.h"
#include "list.h"

class TBody
{
	public:
		TBody(double (*sRotationV)(double Time) = NULL,
				double sRotationAxisX=0, double sRotationAxisY=0);
		~TBody();

		int LoadFromFile(const char* filename);
		void Rotate(double angle);
		bool PointIsValid(Vector p);
		double SurfaceLength();

		TList<TObject> *List;
		bool InsideIsValid;
		bool isInsideValid();
		Vector RotationAxis;
		double (*RotationV)(double Time); double RotationVVar;

		Vector Force; //dont forget to zero it when u want

		//Heat layer
		void CleanHeatLayer();
		int *ObjectIsInHeatLayer(TObject &obj); //link to element of HeatLayer array

		int *HeatLayer;
		double HeatLayerHeight;
};

#endif /* BODY_H_ */
