#include "AtlasGenerator.hpp"
#include "Resources.hpp"
#include "algorithm"
#include "Image.hpp"

using namespace MPACK::Core;
using namespace MPACK::Graphics;


namespace MPACK
{
	namespace Graphics
	{
		int AtlasGenerator::mPadding;

		AtlasGenerator::AtlasGenerator()
		{
		}

		bool AtlasGenerator::generateAtlas(const int widthAtlas, const int heightAtlas, const int padding, const int sortType, const string inputPathJSON, const string outputPath, const string prefix)
		{
			mPadding = padding;

			JSONParser parser;
			DOM *imageJSON;
			vector<ImageProperty*> images;

			Algorithm::SearchList<std::string,DOM*> children = parser.Load(inputPathJSON)->Childs();
			if (children.Size() == 0)
			{
				LOGI("generateAtlas : No image found in JSON");
				return false;
			}

			Algorithm::SearchList<std::string,DOM*>::Iterator it;
			for (it = children.Begin(); it != children.End(); it++)
			{
				imageJSON = it->value;
				string path = it->key;
				int category;
				category = toNumber(imageJSON->GetValue());

				LOGI("path = %s category = %d",path.c_str(),category);

				Graphics::Image *img = new Graphics::Image();
				Core::ReturnValue ans = img->Load(path);

				if (ans != RETURN_VALUE_OK)
				{
					LOGI("Error loading the image at path %s", path.c_str());
					return false;
				}

				int width =  img->GetWidth();
				int height = img->GetHeight();

				ImageProperty *image = new ImageProperty(width, height, path, category);

				img->Unload();
				delete img;
				images.push_back(image);
			}

			return generateAtlas(widthAtlas, heightAtlas, sortType, outputPath, prefix,images);
		}

		bool AtlasGenerator::generateAtlas(const int widthAtlas,const int heightAtlas, const int sortType, const string outputPath, const string prefix, vector<ImageProperty*> & images)
		{
			if (sortType == BEST_OF_ALL)
			{
				vector<AtlasTree*> res[4];
				int min = -1, minInd;
				for (int i = 0; i < 4;i++)
				{
					bool ok = generateAtlasHelper(widthAtlas, heightAtlas, i+1, images, res[i]);
					if (!ok) return false;
					if (min == -1 || min > res[i].size())
					{
						min = res[i].size();
						minInd = i;
					}
				}

				for (int  i = 0; i < res[minInd].size(); i++)
				{
					Graphics::Image *image = new Graphics::Image();
					image->InitColor(widthAtlas, heightAtlas, Color(255,255,255,0));
					DOM *json = new DOM();

					res[minInd][i]->generateJsonAndImage(image, json);

					string path = getPath(outputPath, prefix, i);
					image->Save(path + ".png", Graphics::Image::PNG);
					JSONParser parser;
					parser.Save(path + ".dom", json, JSONParser::STYLE_PRETTY);
				}

				for (int i = 0; i < 4;i++)
					clearVector(res[i]);
			}
			else
			{
				vector<AtlasTree*> res;
				bool ok = generateAtlasHelper(widthAtlas, heightAtlas, sortType, images, res);
				if (!ok) return false;

				for (int  i = 0; i < res.size(); i++)
				{
					Graphics::Image *image = new Graphics::Image();
					image->InitColor(widthAtlas, heightAtlas, Color(255,255,255,0));
					DOM *json = new DOM();

					res[i]->generateJsonAndImage(image, json);

					string path = getPath(outputPath, prefix, i);
					image->Save(path + ".png", Graphics::Image::PNG);
					JSONParser parser;
					parser.Save(path + ".dom", json, JSONParser::STYLE_PRETTY);
				}
				clearVector(res);
			}

			return true;
		}


		bool AtlasGenerator::generateAtlasHelper(const int widthAtlas, const int heightAtlas, const int sortType, vector<ImageProperty*> & images, vector<AtlasTree*> &res)
		{
			switch (sortType)
			{
				case WIDTH:
				{
					sort(images.begin(), images.end(), cmpWidth);
					break;
				}
				case HEIGHT:
				{
					sort(images.begin(), images.end(), cmpHeight);
					break;
				}
				case MAX_SIDE:
				{
					sort(images.begin(), images.end(), cmpMaxSide);
					break;
				}
				case AREA:
				{
					sort(images.begin(), images.end(), cmpArea);
					break;
				}
			}


			for (int i = 0;i < images.size(); i++)
			{
				if (images[i]->width > widthAtlas || images[i]->height > heightAtlas)
				{
					LOGI("Imagine prea mare pentru dimensiunile atlasului!");
					clearVector(res);
					return false;
				}

				if (images[i]->category == -1)
				{
					sort(res.begin(), res.end(), cmpLess);
					bool ok = false;
					for (int j = 0;j < res.size() && !ok;j++)
						ok = res[j]->tryToAdd(images[i]);
					if (!ok)
					{
						AtlasTree *newAtlas = new AtlasTree(0, 0, widthAtlas, heightAtlas);
						res.push_back(newAtlas);
						res[res.size()-1]->tryToAdd(images[i]);
					}
				}
				else
				{
					int j;

					for (j=i;j<images.size() && images[j]->category == images[i]->category;j++);
					j--;

					vector<AtlasTree*> max_atlases;

					for (int k = i; k <= j; k++)
					{
						if (images[k]->width > widthAtlas || images[k]->height > heightAtlas)
						{
							LOGI("Imagine prea mare pentru dimensiunile atlasului!");
							clearVector(res);
							clearVector(max_atlases);
							return false;
						}
						bool ok = false;
						for (int p = 0; !ok && p < max_atlases.size(); p++)
							ok = max_atlases[p]->tryToAdd(images[k]);
						if (!ok)
						{
							AtlasTree *newAtlas = new AtlasTree(0, 0, widthAtlas, heightAtlas);
							max_atlases.push_back(newAtlas);
							max_atlases[max_atlases.size()-1]->tryToAdd(images[k]);
						}
					}

					int left,right = max_atlases.size() - 1,mid,ans = max_atlases.size();
					int maxSize = max_atlases.size();
					if (max_atlases.size() > res.size())
						left = max_atlases.size() - res.size();
					else
						left = 0;

					clearVector(max_atlases);

					while (left <= right)
					{
						mid = (left + right) / 2;

						bool success = true;
						vector<AtlasTree*> aux;
						int k;
						for (k = i; k <= j; k++)
						{
							bool ok = false;
							for (int p = 0; !ok && p < aux.size(); p++)
								ok = aux[p]->tryToAdd(images[k]);
							if (!ok)
							{
								if (aux.size() == mid) break;
								AtlasTree *newAtlas = new AtlasTree(0, 0, widthAtlas, heightAtlas);
								aux.push_back(newAtlas);
								aux[aux.size()-1]->tryToAdd(images[k]);
							}
						}

						int remained = maxSize - mid;
						for (;k <= j;k++)
						{
							bool ok = false;
							for (int p = 0; !ok && p < remained; p++)
								ok = res[p]->tryToAdd(images[k]);
							if (!ok)
							{
								success = false;
								break;
							}
						}

						for (int p = 0;p < remained; p++)
							res[p]->deleteCategory(images[i]->category);
						clearVector(aux);

						if (success)
						{
							ans = mid;
							right = mid-1;
						}
						else
							left = mid+1;
					}

					int k;
					int backInd = res.size();
					for (k = i; k <= j; k++)
					{
						bool ok = false;
						for (int p = backInd; !ok && p < res.size(); p++)
							ok = res[p]->tryToAdd(images[k]);
						if (!ok)
						{
							if (res.size() - backInd  == ans) break;
							AtlasTree *newAtlas = new AtlasTree(0, 0, widthAtlas, heightAtlas);
							res.push_back(newAtlas);
							res[res.size()-1]->tryToAdd(images[k]);
						}
					}

					for (;k <= j;k++)
					{
						bool ok = false;
						int remained = maxSize - ans;
						for (int p = 0; !ok && p < remained; p++)
							ok = res[p]->tryToAdd(images[k]);
					}
					i = j;
				}
			}

			return true;
		}

		AtlasGenerator::AtlasTree::AtlasTree(){}
		AtlasGenerator::AtlasTree::AtlasTree(int X,int Y, int W, int H):x(X),y(Y),width(W),height(H),busy(false),left(0),right(0)
		{
			freeSize = width * height;
		}

		AtlasGenerator::AtlasTree::~AtlasTree()
		{
			if (left)
			{
				delete left;
				left = 0;
			}
			if (right)
			{
				delete right;
				right = 0;
			}
		}

		void AtlasGenerator::AtlasTree::deleteCategory(int Category)
		{
			if (busy && category == Category)
			{
				if (left)
				{
					delete left;
					left = 0;
				}
				if (right)
				{
					delete right;
					right = 0;
				}
				busy = category = 0;
				freeSize = width * height;
			}
			else
			{
				if (left) left->deleteCategory(Category);
				if (right) right->deleteCategory(Category);
				if (left || right)
				{
					freeSize = 0;
					if (left) freeSize += left->freeSize;
					if (right) freeSize += right->freeSize;
				}
			}
		}

		bool AtlasGenerator::AtlasTree::hasCategory(int Category)
		{
			if (busy && category == Category || (left && left->hasCategory(Category)) || (right && right->hasCategory(Category)))
				return true;
			else
				return false;
		}
		bool AtlasGenerator::AtlasTree::tryToAdd(ImageProperty *img)
		{
			LOGI("width = %d height = %d x = %d y = %d busy = %d",width, height,x,y, busy);
			if (!busy)
			{
				if (width >= img->width && height >= img->height)
				{
					busy = true;
					category = img->category;
					path = img->path;
					imageWidth = img->width;
					imageHeight = img->height;
					if (width - img->width > mPadding)
					{
						left = new AtlasTree(x + img->width + mPadding, y, width - img->width - mPadding, img->height);
					}
					if (height - img->height > mPadding)
					{
						right = new AtlasTree(x, y + img->height + mPadding, width, height - img->height - mPadding);
					}
					freeSize = 0;
					if (left) freeSize += left->freeSize;
					if (right) freeSize += right->freeSize;

					return true;
				}
			}
			else
			{
				if (left && left->tryToAdd(img))
					return true;
				if (right && right->tryToAdd(img))
					return true;
			}
			return false;
		}

		void AtlasGenerator::AtlasTree::generateJsonAndImage(Graphics::Image * canvas,  DOM * canvasJson)
		{
			Graphics::Image *img;
			if (busy)
			{
				img = new Graphics::Image();
				img->Load(path);

				canvas->Blit(img, Math::Vector2<int>(x, y));
				DOM* newDom = new DOM();
				newDom->AddString("x", toString(x));
				newDom->AddString("y", toString(y));
				newDom->AddString("width", toString(imageWidth));
				newDom->AddString("height", toString(imageHeight));
				canvasJson->Childs().PushBack(path, newDom);

				img->Unload();
				delete img;
			}
			if (left) left->generateJsonAndImage(canvas, canvasJson);
			if (right) right->generateJsonAndImage(canvas, canvasJson);
		}

		bool AtlasGenerator::cmpWidth(ImageProperty *x, ImageProperty *y)
		{
			if (x->category == y->category)
				return x->width > y->width;
			else
			{
				if (x->category == -1)
					return false;
				if (y->category == -1)
					return true;
				return (x->category < y->category);
			}
		}

		bool AtlasGenerator::cmpHeight(ImageProperty *x, ImageProperty *y)
		{
			if (x->category == y->category)
				return x->height > y->height;
			else
			{
				if (x->category == -1)
					return false;
				if (y->category == -1)
					return true;
				return (x->category < y->category);
			}
		}

		bool AtlasGenerator::cmpMaxSide(ImageProperty *x, ImageProperty *y)
		{
			if (x->category == y->category)
				return max(x->width, x->height) > max(y->width, y->height);
			else
			{
				if (x->category == -1)
					return false;
				if (y->category == -1)
					return true;
				return (x->category < y->category);
			}
		}

		bool AtlasGenerator::cmpArea(ImageProperty *x, ImageProperty *y)
		{
			if (x->category == y->category)
				return x->width * x->height > y->width * y->height;
			else
			{
				if (x->category == -1)
					return false;
				if (y->category == -1)
					return true;
				return (x->category < y->category);
			}
		}

		bool AtlasGenerator::cmpLess(AtlasTree *x, AtlasTree *y)
		{
			return x->freeSize < y->freeSize;
		}

		bool AtlasGenerator::cmpBigger(AtlasTree *x, AtlasTree *y)
		{
			return x->freeSize > y->freeSize;
		}


		void AtlasGenerator::clearVector(vector<AtlasTree*> &v)
		{
			for (int i = 0; i < v.size(); i++)
				delete v[i];
			v.clear();
		}

		int AtlasGenerator::toNumber(const string s)
		{
			int semn = 1;
			int i = 0,res = 0;
			if (s[0] == '-')
			{
				semn = -1;
				i++;
			}

			for (;i<s.size();i++)
				res = res * 10 + s[i]-'0';
			return res * semn;
		}

		string AtlasGenerator::toString(const int number)
		{
			int x = number;
			string ans = "";
			if (x == 0)
			{
				ans = "0";
				return ans;
			}

			while (x)
			{
				ans =  (char)( '0'+x%10) +  ans;
				x/=10;
			}
			return ans;
		}

		string AtlasGenerator::getPath(const string outputPath, const string prefix, const int index)
		{
			return  outputPath + "/" + prefix + toString(index);
		}
	}
}