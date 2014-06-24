#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>
#include "hdf5.h"

#include "flowmove.h"
#include "epsfast.h"

const double ExpArgRestriction = -8.;
double dl;
double EPS_MULT;
using namespace std;

class printer
{
	public:
		printer() {last=0;}
		void go(int percent)
		{
			if (clock()-last < CLOCKS_PER_SEC) return;
			fprintf(stderr, "%3d%%\r", percent);
			fflush(stderr);
			last = clock();
		}
	private:
		clock_t last;
};

bool PointIsInvalid(Space *S, TVec p)
{
	const_for(S->BodyList, llbody)
	{
		if ((**llbody).isPointInvalid(p)) return true;
	}
	return false;
}

double eps2h(const TSortedNode &Node, TVec p)
{
	TVec dr;
	double res1, res2;
	res2 = res1 = DBL_MAX;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		for (TObj *lobj = nnode.vRange.first; lobj < nnode.vRange.last; lobj++)
		{
			dr = p - lobj->r;
			double drabs2 = dr.abs2();
			if ( drabs2 ) {
			if ( res1 > drabs2 )
			{
				res2 = res1;
				res1 = drabs2;
			}
			else if ( res2 > drabs2 )
			{
				res2 = drabs2;
			}}
		}
		#undef nnode
	}

	if ( res1 == DBL_MAX ) return DBL_MIN;
	if ( res2 == DBL_MAX ) return DBL_MIN;//res1;

	return res2;
}

TObj* Nearest(TSortedNode &Node, TVec p)
{
	TVec dr(0, 0);
	double resr = DBL_MAX;
	TObj *res = NULL;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		for (TObj *lobj = nnode.vRange.first; lobj < nnode.vRange.last; lobj++)
		{
			dr = p - lobj->r;
			double drabs2 = dr.abs2();
			if ( !drabs2 ) continue;
			if ( drabs2 <= resr )
			{
				res = lobj;
				resr = drabs2;
			}
		}
		#undef nnode
	}

	return res;
}

double h2(TSortedNode &Node, TVec p)
{
	double resh2 = DBL_MAX;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		auto blist = nnode.BodyLList;
		if ( !blist ) { continue; }
		const_for (blist, llobj)
		{
			resh2 = min(resh2, (p-(**llobj).r).abs2());
		}
		#undef nnode
	}

	return resh2;
}

double Vorticity(Space* S, TVec p)
{
	double T=0;
	auto *hlist = S->VortexList;
	TSortedNode* bnode = S->Tree->findNode(p);

	//return  bnode->NearNodes->size_safe();
	TObj* nrst = Nearest(*bnode, p);

	//return nrst - S->HeatList->begin();

	const_for(bnode->NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		for (TObj *lobj = nnode.vRange.first; lobj < nnode.vRange.last; lobj++)
		{
			double exparg = -(p-lobj->r).abs2() * lobj->v.x; // v.rx stores eps^(-2)
			T+= (exparg>-10) ? lobj->v.y * exp(exparg) : 0; // v.ry stores g*eps(-2)
		}
		#undef nnode
	}

	T*= C_1_PI;

	double erfarg = h2(*bnode, p)/sqr(dl*EPS_MULT);
	T+= (erfarg<3) ? 0.5*(1-erf(erfarg)) : 0;

	return T;
}

static hid_t DATASPACE_SCALAR;
void attribute_write(hid_t hid, const char *name, float value)
{
	if (value == 0) return;
	hid_t aid = H5Acreate2(hid, name, H5T_NATIVE_FLOAT, DATASPACE_SCALAR, H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, H5T_NATIVE_FLOAT, &value)>=0);
	assert(H5Aclose(aid)>=0);
}

int main(int argc, char *argv[])
{
	if ( argc != 7)\
	{
		fprintf(stderr, "Usage: vvmap ", argv[0]);
		fprintf(stderr, "[FILE] xmin xmax ymin ymax spacing\n");
		fprintf(stderr, "vvmap calculates stream function distribution on a regular mesh\n");
		fprintf(stderr, "Environment:\n\tVV_EPS_MULT - Vortexes smoothing coefficient. Default 2.\n");
		return -1;
	}

	char *mult_env = getenv("VV_EPS_MULT");
	EPS_MULT = mult_env ? atof(mult_env) : 2;

	Space *S = new Space();
	S->Load(argv[1]);
	flowmove fm(S);
	fm.VortexShed();
	printer my_printer;
	S->HeatList = NULL;

	dl = S->AverageSegmentLength();
	S->Tree = new TSortedTree(S, 8, dl*20, DBL_MAX);
	S->Tree->build();

	#pragma omp parallel for
	const_for(S->Tree->getBottomNodes(), llbnode)
	{
		for (TObj *lobj = (**llbnode).vRange.first; lobj < (**llbnode).vRange.last; lobj++)
		{
			lobj->v.x = 1./(sqr(EPS_MULT)*max(eps2h(**llbnode, lobj->r), sqr(0.6*dl)));
			lobj->v.y = lobj->v.x * lobj->g;
		}
	}

	/************** READ ARGUMENTS ****************/
	double xmin = atof(argv[2]);
	double xmax = atof(argv[3]);
	double ymin = atof(argv[4]);
	double ymax = atof(argv[5]);
	double spacing = atof(argv[6]);
	/******************************************/

	int total = int((xmax-xmin)/spacing + 1)*int((ymax-ymin)/spacing + 1);
	int now=0;

	// Calculate field ********************************************************
	hsize_t dims[2] = {(xmax-xmin)/spacing + 1, (ymax-ymin)/spacing + 1};
	float *mem = (float*)malloc(sizeof(float)*dims[0]*dims[1]);

	for (int xi=0; xi<dims[0]; xi++)
	{
		double x = xmin + double(xi)*spacing;
		#pragma omp parallel for ordered schedule(dynamic, 100)
		for( int yj=0; yj<dims[1]; yj++)
		{
			double y = ymin + double(yj)*spacing;
			TVec xy(x,y);
			mem[xi*dims[1]+yj] = PointIsInvalid(S, xy) ? 0 : Vorticity(S, xy);

			#pragma omp critical
			my_printer.go(++now*100/total);
		}
	}


	// Create HDF file
	char fname[128]; sprintf(fname, "%s.maps", argv[1]);
	hid_t fid = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	assert(fid>=0);

	// Create dataspace and dataset
	DATASPACE_SCALAR = H5Screate(H5S_SCALAR);
	hid_t file_dataspace = H5Screate_simple(2, dims, dims);
	assert(file_dataspace>=0);
	hid_t file_dataset = H5Dcreate2(fid, "vorticity", H5T_NATIVE_FLOAT, file_dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	assert(file_dataset>=0);

	attribute_write(file_dataset, "xmin", xmin);
	attribute_write(file_dataset, "xmax", xmax);
	attribute_write(file_dataset, "ymin", ymin);
	attribute_write(file_dataset, "ymax", ymax);
	attribute_write(file_dataset, "spacing", spacing);
	
	H5Dwrite(file_dataset, H5T_NATIVE_FLOAT, H5S_ALL, file_dataspace, H5P_DEFAULT, mem);
	H5Dclose(file_dataset);
	H5Sclose(DATASPACE_SCALAR);
	H5Sclose(file_dataspace);
	H5Fclose(fid);
	free(mem);

	return 0;
}
