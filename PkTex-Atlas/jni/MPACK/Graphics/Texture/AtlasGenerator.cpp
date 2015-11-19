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
    const double AtlasGenerator::threshold = (double)20/100;

		AtlasGenerator::AtlasGenerator()
		{
		}

		bool AtlasGenerator::generateAtlas(const int widthAtlas, const int heightAtlas, const int padding, const string inputPathJSON, const string outputPath, const string prefix, const int sortType)
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

				Graphics::Image *img = new Graphics::Image();
				Core::ReturnValue ans = img->Load(path, false);

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

			bool ok = generateAtlas(widthAtlas, heightAtlas, sortType, outputPath, prefix, images);
			clearImagePropertyVector(images);
			LOGI("Atlases created!");
			return ok;
		}

		bool AtlasGenerator::generateAtlas(const int widthAtlas,const int heightAtlas, const int sortType, const string outputPath, const string prefix, vector<ImageProperty*> & images)
		{
			if (sortType == BEST_OF_ALL)
			{
				vector<AtlasTree*> res[4];
				int minImages = -1, minSize = -1, minInd, totalSize;

				for (int i = 0; i < 4;i++)
				{
					bool ok = generateAtlasHelper(widthAtlas, heightAtlas, i+1, images, res[i]);
					if (!ok) return false;
					resizeImages(widthAtlas, heightAtlas, res[i]);

					totalSize = 0;
					for (int j = 0;j < res[i].size(); j++)
						totalSize += res[i][j]->width * res[i][j]->height;

					if (minImages == -1 || minImages > res[i].size() || (minImages == res[i].size() && minSize > totalSize))
					{
						minSize = totalSize;
						minImages = res[i].size();
						minInd = i;
					}
				}

				DOM *hashJson = new DOM();
				for (int  i = 0; i < res[minInd].size(); i++)
				{
					Graphics::Image *image = new Graphics::Image();
					image->InitColor(res[minInd][i]->width, res[minInd][i]->height, Color(255,255,255,0));
					DOM *json = new DOM();

					res[minInd][i]->generateJsonAndImage(image, json);

					string path = getPath(outputPath, prefix, i);
					image->Save(path + ".png", Graphics::Image::PNG);
					JSONParser parser;
					parser.Save(path + ".dom", json, JSONParser::STYLE_PRETTY);
					hashJson->AddString(path+".dom", path+".png");
					delete json;
					delete image;
				}
				JSONParser parser;
				parser.Save(outputPath+"/"+prefix+".dom", hashJson);
				delete hashJson;

				for (int i = 0; i < 4;i++)
					clearAtlasTreeVector(res[i]);
			}
			else
			{
				vector<AtlasTree*> res;
				bool ok = generateAtlasHelper(widthAtlas, heightAtlas, sortType, images, res);
				if (!ok) return false;

				resizeImages(widthAtlas, heightAtlas, res);

				DOM *hashJson = new DOM();
				for (int  i = 0; i < res.size(); i++)
				{
					Graphics::Image *image = new Graphics::Image();
					image->InitColor(res[i]->width, res[i]->height, Color(255,255,255,0));
					DOM *json = new DOM();

					res[i]->generateJsonAndImage(image, json);

					string path = getPath(outputPath, prefix, i);
					hashJson->AddString(path+".dom", path+".png");
					image->Save(path + ".png", Graphics::Image::PNG);
					JSONParser parser;
					parser.Save(path + ".dom", json, JSONParser::STYLE_PRETTY);
					delete json;
					delete image;
				}
				JSONParser parser;
				parser.Save(outputPath+"/"+prefix+".dom", hashJson);
				delete hashJson;
				clearAtlasTreeVector(res);
			}

			return true;
		}

    bool AtlasGenerator::generateAtlasHelper(const int widthAtlas, const int heightAtlas, const int sortType, vector<ImageProperty*> & images, vector<AtlasTree*> &res)
    {
      for (int i = 0; i < images.size(); i++)
      {
        if (images[i]->width > widthAtlas || images[i]->height > heightAtlas)
        {
          LOGI("Image is bigger than the atlas. Choose better dimensions for the atlas.");
          clearAtlasTreeVector(res);
          return false;
        }
      }

      switch (sortType)
      {
        case WIDTH:
        {
          sort(images.begin(), images.end(), cmpCategoryWidth);
          break;
        }
        case HEIGHT:
        {
          sort(images.begin(), images.end(), cmpCategoryHeight);
          break;
        }
        case MAX_SIDE:
        {
          sort(images.begin(), images.end(), cmpCategoryMaxSide);
          break;
        }
        case AREA:
        {
          sort(images.begin(), images.end(), cmpCategoryArea);
          break;
        }
      }

      vector<ImageProperty*> remainingImages;
      vector<int> atlasInd (images.size(),-1);

      for (int i = 0; i < images.size(); i++)
      {
        if (images[i]->category == -1)
        {
          remainingImages.push_back(images[i]);
          continue;
        }
        int category = images[i]->category;
        int j,k,p;
        vector<AtlasTree*> categoryAtlas;
        vector<bool> categoryHelper;

        for (j = i; j < images.size() && images[j]->category == category; j++)
        {
          bool ok = false;
          for (k = 0; !ok && k < categoryAtlas.size(); k++)
          {
            ok = categoryAtlas[k]->tryToAdd(images[j]);
            if (ok)
              atlasInd[j] = k;
          }

          if (!ok)
          {
            AtlasTree *newAtlas = new AtlasTree(0, 0, widthAtlas, heightAtlas);
            categoryAtlas.push_back(newAtlas);
            categoryHelper.push_back(false);
            categoryAtlas[categoryAtlas.size()-1]->tryToAdd(images[j]);
            atlasInd[j] = categoryAtlas.size()-1;
          }
        }

        for (k = 0; k < categoryAtlas.size(); k++)
        {
          if ( (double)categoryAtlas[k]->freeSize  / (categoryAtlas[k]->width * categoryAtlas[k]->height)  <= threshold )
          {
            categoryHelper[k] = true;
            res.push_back(categoryAtlas[k]);
          }
          else
            delete categoryAtlas[k];
        }
        categoryAtlas.clear();

        for (k = i; k < j; k++)
          if (atlasInd[k] == -1 || !categoryHelper[atlasInd[k]])
          {
            remainingImages.push_back(images[k]);
            atlasInd[k] = -1;
          }
        categoryHelper.clear();
        i = j-1;
      }

      switch (sortType)
      {
        case WIDTH:
        {
          sort(remainingImages.begin(), remainingImages.end(), cmpWidth);
          break;
        }
        case HEIGHT:
        {
          sort(remainingImages.begin(), remainingImages.end(), cmpHeight);
          break;
        }
        case MAX_SIDE:
        {
          sort(remainingImages.begin(), remainingImages.end(), cmpMaxSide);
          break;
        }
        case AREA:
        {
          sort(remainingImages.begin(), remainingImages.end(), cmpArea);
          break;
        }
      }

      for (int i = 0; i < remainingImages.size(); i++)
        if (remainingImages[i]->category == -1)
        {
          sort(res.begin(), res.end(), cmpLess);

          bool ok = false;
          for (int j = 0; !ok && j < res.size(); j++)
            ok = res[j]->tryToAdd(remainingImages[i]);
          if (!ok)
          {
            AtlasTree *newAtlas = new AtlasTree(0, 0, widthAtlas, heightAtlas);
            res.push_back(newAtlas);
            res[res.size()-1]->tryToAdd(remainingImages[i]);
          }
        }
        else
        {
          sort(res.begin(), res.end(), cmpLess);

          bool ok = false;
          for (int j = 0; !ok && j < res.size(); j++)
            if (res[j]->hasCategory(remainingImages[i]->category))
              ok = res[j]->tryToAdd(remainingImages[i]);

          for (int j = 0; !ok && j < res.size(); j++)
            ok = res[j]->tryToAdd(remainingImages[i]);

          if (!ok)
          {
            AtlasTree *newAtlas = new AtlasTree(0, 0, widthAtlas, heightAtlas);
            res.push_back(newAtlas);
            res[res.size()-1]->tryToAdd(remainingImages[i]);
          }
        }

      remainingImages.clear();

      return true;
    }


		bool AtlasGenerator::generateAtlasHelperOld(const int widthAtlas, const int heightAtlas, const int sortType, vector<ImageProperty*> & images, vector<AtlasTree*> &res)
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
          LOGI("Image is bigger than the atlas. Choose better dimensions for the atlas.");
					clearAtlasTreeVector(res);
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
							clearAtlasTreeVector(res);
							clearAtlasTreeVector(max_atlases);
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

					clearAtlasTreeVector(max_atlases);

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
						clearAtlasTreeVector(aux);

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


		void AtlasGenerator::resizeImages(const int widthAtlas, const int heightAtlas, vector<AtlasTree*> &res)
		{
			int i;
			int maxRange = 0;

			while ( (1 << (maxRange+1)) <= widthAtlas && (1 << (maxRange+1)) <= heightAtlas ) maxRange++;


			for (int i = 0; i < res.size(); i++)
			{
				int left = 0, right = maxRange, mid, ans = -1;
				vector <ImageProperty*> images;
				res[i]->getImagesVector(images);

				while (left <= right)
				{
					mid = (left + right) / 2;

					AtlasTree *atlas = new AtlasTree(0, 0, 1<<mid, 1<<mid);
					bool ok = true;
					for (int j = 0;ok && j < images.size(); j++)
						ok = atlas->tryToAdd(images[j]);

					if (ok)
					{
						if (ans == -1 || ans > mid)
							ans = mid;
						right = mid-1;
					}
					else
						left = mid+1;

					delete atlas;
				}

				if (ans != -1)
				{
					delete res[i];
					res[i] = new AtlasTree(0, 0, 1<<ans, 1<<ans);
					for (int j = 0;j < images.size(); j++)
						res[i]->tryToAdd(images[j]);
				}

				clearImagePropertyVector(images);
			}

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
        bool ok = false;
				if (left && left->tryToAdd(img))
					ok = true;
				if (!ok && right && right->tryToAdd(img))
					ok = true;
        freeSize = 0;
        if (left) freeSize += left->freeSize;
        if (right) freeSize += right->freeSize;
        return ok;
      }
			return false;
		}

		void AtlasGenerator::AtlasTree::generateJsonAndImage(Graphics::Image * canvas,  DOM * canvasJson)
		{
			Graphics::Image *img;
			if (busy)
			{
				img = new Graphics::Image();
				img->Load(path, false);

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

		void AtlasGenerator::AtlasTree::getImagesVector(vector<ImageProperty*> &res)
		{
			if (busy)
			{
				ImageProperty *img = new ImageProperty(imageWidth, imageHeight, path, category);
				res.push_back(img);
			}
			if (left) left->getImagesVector(res);
			if (right) right->getImagesVector(res);
		}

		bool AtlasGenerator::cmpWidth(ImageProperty *x, ImageProperty *y)
		{
			if (x->width == y->width)
      {
          if (x->category == -1)
            return false;
          if (y->category == -1)
            return true;
          return (x->category < y->category);
      }
			return x->width > y->width;
		}

		bool AtlasGenerator::cmpHeight(ImageProperty *x, ImageProperty *y)
		{
			if (x->height == y->height)
      {
          if (x->category == -1)
            return false;
          if (y->category == -1)
            return true;
          return (x->category < y->category);
      }
      return x->height > y->height;
		}

		bool AtlasGenerator::cmpMaxSide(ImageProperty *x, ImageProperty *y)
		{
			if (max(x->width, x->height) == max(y->width, y->height))
      {
          if (x->category == -1)
            return false;
          if (y->category == -1)
            return true;
          return (x->category < y->category);
      }
      return max(x->width, x->height) > max(y->width, y->height);
		}

		bool AtlasGenerator::cmpArea(ImageProperty *x, ImageProperty *y)
		{
      if (x->width * x->height == y->width * y->height)
      {
          if (x->category == -1)
            return false;
          if (y->category == -1)
            return true;
          return (x->category < y->category);
      }
      return x->width * x->height > y->width * y->height;
		}

    bool AtlasGenerator::cmpCategoryWidth(ImageProperty *x, ImageProperty *y)
    {
      if (x->category != y->category)
      {
          if (x->category == -1)
            return false;
          if (y->category == -1)
            return true;
          return (x->category < y->category);
      }
      return x->width > y->width;
    }

    bool AtlasGenerator::cmpCategoryHeight(ImageProperty *x, ImageProperty *y)
    {
      if (x->category != y->category)
      {
          if (x->category == -1)
            return false;
          if (y->category == -1)
            return true;
          return (x->category < y->category);
      }
      return x->height > y->height;
    }

    bool AtlasGenerator::cmpCategoryMaxSide(ImageProperty *x, ImageProperty *y)
    {
      if (x->category != y->category)
      {
          if (x->category == -1)
            return false;
          if (y->category == -1)
            return true;
          return (x->category < y->category);
      }
      return max(x->width, x->height) > max(y->width, y->height);
    }

    bool AtlasGenerator::cmpCategoryArea(ImageProperty *x, ImageProperty *y)
    {
      if (x->category != y->category)
      {
          if (x->category == -1)
            return false;
          if (y->category == -1)
            return true;
          return (x->category < y->category);
      }
      return x->width * x->height > y->width * y->height;
    }

		bool AtlasGenerator::cmpLess(AtlasTree *x, AtlasTree *y)
		{
			return x->freeSize < y->freeSize;
		}

		bool AtlasGenerator::cmpBigger(AtlasTree *x, AtlasTree *y)
		{
			return x->freeSize > y->freeSize;
		}


		void AtlasGenerator::clearAtlasTreeVector(vector<AtlasTree*> &v)
		{
			for (int i = 0; i < v.size(); i++)
				delete v[i];
			v.clear();
		}

		void AtlasGenerator::clearImagePropertyVector(vector<ImageProperty*> &v)
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
