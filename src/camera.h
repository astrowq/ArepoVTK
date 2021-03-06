/*
 * camera.h
 * dnelson
 */
 
#ifndef AREPO_RT_CAMERA_H
#define AREPO_RT_CAMERA_H

#include "ArepoRT.h"

class Filter {
public:
  // construction
  virtual ~Filter();
  Filter(float xw, float yw)
      : xWidth(xw), yWidth(yw), invXWidth(1.f/xw), invYWidth(1.f/yw) {
  }
  // methods
  virtual float Evaluate(float x, float y) const = 0;

  // data
  const float xWidth, yWidth;
  const float invXWidth, invYWidth;
};

class BoxFilter : public Filter {
public:
  BoxFilter(float xw, float yw) : Filter(xw, yw) { 
      IF_DEBUG(cout << "BoxFilter(" << xw << ", " << yw << ") constructor." << endl);
  }
  float Evaluate(float x, float y) const;
};

BoxFilter *CreateBoxFilter();

class Film {
public:
  // construction
  Film(int xres, int yres)
      : xResolution(xres), yResolution(yres) {
      IF_DEBUG(cout << "Film(" << xres << ", " << yres << ") constructor." << endl);
  }
  Film(int xres, int yres, Filter *filt, const double crop[4],
            const string &filename, bool openWindow);
  ~Film() {
      delete pixels;
      delete integrals;
      delete filter;
      delete[] filterTable;
  }
  
  // methods
  void AddSample(const CameraSample &sample, const Spectrum &L, const Ray &ray, int threadNum);
  void Splat(const CameraSample &sample, const Spectrum &L);
  void GetSampleExtent(int *xstart, int *xend, int *ystart, int *yend) const;
  void GetPixelExtent(int *xstart, int *xend, int *ystart, int *yend) const;
  void UpdateDisplay(int x0, int y0, int x1, int y1, float splatScale = 1.f);
  void WriteImage(int frameNum, float splatScale = 1.f);
  void WriteIntegrals();
  void WriteRawRGB();
  
  void CalculateScreenWindow(float *screen, int jobNum);
  bool DrawLine(float x1, float y1, float x2, float y2, const Spectrum &L);

  // data
  const int xResolution, yResolution;
private:
  Filter *filter;
  double cropWindow[4];
  string filename;
  int xPixelStart, yPixelStart, xPixelCount, yPixelCount;
  
  struct Pixel {
      Pixel() {
        for (int i = 0; i < 3; ++i)
          Lxyz[i] = splatXYZ[i] = 0.0f;
        weightSum = 0.0f;
      }
      float Lxyz[3];
      float weightSum;
      float splatXYZ[3];
      float pad;
  };
  struct RawPixel {
    RawPixel() {
      for (int i = 0; i < TF_NUM_VALS; ++i)
        raw_vals[i] = 0.0f;
      weightSum = 0.0f;
    }
    float raw_vals[TF_NUM_VALS];
    float weightSum;
  };
  
  BlockedArray<Pixel> *pixels;
  BlockedArray<RawPixel> *integrals;
  float *filterTable;
};

Film *CreateFilm(Filter *filter);

class Camera {
public:
  // construction
  Camera(const Transform &cam2world, float sopen, float sclose, Film *film);
  Camera(const Transform &cam2world, const Transform &proj, const float screenWindow[4],
         float sopen, float sclose, float lensr, float focald, Film *film);
  virtual ~Camera();
  
  // methods
  virtual float GenerateRay(const CameraSample &sample, Ray *ray) const = 0;
  
  virtual bool RasterizeLine(const Point &p1, const Point &p2, const Spectrum &L) const = 0;

  // data
  Transform CameraToWorld;
  const float shutterOpen, shutterClose;
  Film *film;

protected:
  // private data
  Transform CameraToScreen, RasterToCamera;
  Transform ScreenToRaster, RasterToScreen;
  Transform WorldToRaster;
  float lensRadius, focalDistance;
};

class OrthoCamera : public Camera {
public:
  // methods
  OrthoCamera(const Transform &cam2world, const float screenWindow[4],
              float sopen, float sclose, float lensr, float focald, Film *film);
  float GenerateRay(const CameraSample &sample, Ray *) const;
  bool RasterizeLine(const Point &p1, const Point &p2, const Spectrum &L) const;
  
private:
  // data
  Vector dxCamera, dyCamera;
};

class PerspectiveCamera : public Camera {
public:
  // methods
  PerspectiveCamera(const Transform &cam2world, const float screenWindow[4],
                    float sopen, float sclose, float lensr, float focald, float fov, Film *film);
  float GenerateRay(const CameraSample &sample, Ray *) const;
  bool RasterizeLine(const Point &p1, const Point &p2, const Spectrum &L) const;
  
private:
  // data
  Vector dxCamera, dyCamera;
};

class FisheyeCamera : public Camera {
public:
  // methods
  FisheyeCamera(const Transform &cam2world, float sopen, float sclose, Film *film);
  float GenerateRay(const CameraSample &sample, Ray *) const;
  bool RasterizeLine(const Point &p1, const Point &p2, const Spectrum &L) const;
};

class EnvironmentCamera : public Camera {
public:
  // methods
  EnvironmentCamera(const Transform &cam2world, float sopen, float sclose, Film *film);
  float GenerateRay(const CameraSample &sample, Ray *) const;
  bool RasterizeLine(const Point &p1, const Point &p2, const Spectrum &L) const;
};

Camera *CreateCamera(const Transform &cam2world, Film *film);
OrthoCamera *CreateOrthographicCamera(const Transform &cam2world, Film *film);
PerspectiveCamera *CreatePerspectiveCamera(const Transform &cam2world, Film *film);
FisheyeCamera *CreateFisheyeCamera(const Transform &cam2world, Film *film);
EnvironmentCamera *CreateEnvironmentCamera(const Transform &cam2world, Film *film);

#endif
