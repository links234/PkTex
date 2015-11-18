#ifndef ATLAS_GENERATOR
#define ATLAS_GENERATOR

#include "Resources.hpp"
#include "algorithm"
#include "Image.hpp"
#include <vector>
#include <cstring>
#include <string>
using namespace std;

using namespace MPACK::Core;
using namespace MPACK::Graphics;

namespace MPACK
{
	namespace Graphics
	{
		class AtlasGenerator
		{
			public:

			static const int WIDTH = 1;
			static const int HEIGHT = 2;
			static const int MAX_SIDE = 3;
			static const int AREA = 4;
			static const int BEST_OF_ALL = 5;

			private:

			static int mPadding ;

			struct ImageProperty
			{
				int width,height;
				string path;
				int category;

				ImageProperty();
				ImageProperty(const int w,const int h, const string p, const int c):width(w),height(h),path(p),category(c){}
			};

			struct AtlasTree
			{
				int x, y;
				int width, height;
				int imageWidth, imageHeight;
				bool busy;
				int category;
				int freeSize;
				string path;
				AtlasTree *left, *right;

				AtlasTree();
				AtlasTree(int X,int Y, int W, int H);

				~AtlasTree();
				void deleteCategory(int Category);
				bool hasCategory(int Category);
				bool tryToAdd(ImageProperty *img);
				void generateJsonAndImage(Graphics::Image * canvas,  DOM * canvasJson);
			};

			public:
				AtlasGenerator();
				static bool generateAtlas(const int widthAtlas, const int heightAtlas, const int padding, const int sortType, const string inputPathJSON, const string outputPath, const string prefix);

			private:
				static bool generateAtlas(const int widthAtlas, const int heightAtlas, const int sortType, const string outputPath, const string prefix, vector<ImageProperty*> & images);
				static bool generateAtlasHelper(const int widthAtlas, const int heightAtlas, const int sortType, vector<ImageProperty*> & images, vector<AtlasTree*> &res);
				static int toNumber(const string s);
				static string toString(const int number);
				static void clearVector(vector<AtlasTree*> &v);
				static string getPath(const string outputPath, const string prefix, const int index);

				static bool cmpWidth(ImageProperty *x, ImageProperty *y);
				static bool cmpHeight(ImageProperty *x, ImageProperty *y);
				static bool cmpMaxSide(ImageProperty *x, ImageProperty *y);
				static bool cmpArea(ImageProperty *x, ImageProperty *y);
				static bool cmpLess(AtlasTree *x, AtlasTree *y);
				static bool cmpBigger(AtlasTree *x, AtlasTree *y);
		};
	}
}










#endif
