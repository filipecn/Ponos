#include "core/sampler.h"

namespace helios {

	Sampler::Sampler(int xstart, int xend, int ystart, int yend, int spp, float sopen, float sclose)
	: xPixelStart(xstart), xPixelEnd(xend), yPixelStart(ystart), yPixelEnd(yend),
	samplesPerPixel(spp), shutterOpen(sopen), shutterClose(sclose) {}

	bool Sampler::reportResults(Sample *samples, const RayDifferential *rays, const Spectrum *Ls, const Intersection *isects, int count) {
		return true;
	}

	void Sampler::computeSubWindow(int num, int count, int *newXStart, int *newXEnd, int *newYStart, int *newYEnd) const {
		// Determine how many tiles to use in each dimension, nx, and ny
		int dx = xPixelEnd - xPixelStart, dy = yPixelEnd - yPixelStart;
		int nx = count, ny = 1;
		while((nx & 0x1) == 0 && 2 * dx * ny < dy * nx) {
			nx >>= 1;
			ny <<= 1;
		}
		// Compute x and y pixel sample range for sub-window
		int xo = num % nx, yo = num / nx;
		float tx0 = float(xo) / float(nx), tx1 = float(xo + 1) / float(nx);
		float ty0 = float(yo) / float(ny), ty1 = float(yo + 1) / float(ny);
		*newXStart = ponos::floor2Int(ponos::lerp(tx0, xPixelStart, xPixelEnd));
		*newXEnd   = ponos::floor2Int(ponos::lerp(tx1, xPixelStart, xPixelEnd));
		*newYStart = ponos::floor2Int(ponos::lerp(ty0, yPixelStart, yPixelEnd));
		*newYEnd   = ponos::floor2Int(ponos::lerp(ty1, yPixelStart, yPixelEnd));
	}

	Sample::Sample(Sampler *sampler, SurfaceIntegrator *surf, VolumeIntegrator *vol, const Scene *scene) {
		//if(surf) surf->requestSamples(sampler, this, scene);
		//if(vol) vol->requestSamples(sampler, this, scene);
		allocateSampleMemory();
	}

	void Sample::allocateSampleMemory() {
		// Allocate storage for sample pointers
		int np = n1D.size() + n2D.size();
		if(!np) {
			oneD = twoD = nullptr;
			return;
		}
		oneD = ponos::allocAligned<float *>(np);
		twoD = oneD + n1D.size();
		// Compute total number of sample values needed
		int totalSamples = 0;
		for(uint32 i : n1D) totalSamples += i;
		for(uint32 i : n2D) totalSamples += i;
		// Allocate storage for sample values
		float *mem = ponos::allocAligned<float>(totalSamples);
		for(uint32 i = 0; i < n1D.size(); i++) {
			oneD[i] = mem;
			mem += n1D[i];
		}
		for(uint32 i = 0; i < n2D.size(); i++) {
			twoD[i] = mem;
			mem += n2D[i];
		}
	}
} // helios namespace
