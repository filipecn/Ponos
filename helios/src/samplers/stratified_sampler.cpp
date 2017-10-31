#include "samplers/stratified_sampler.h"

namespace helios {

			StratifiedSampler::StratifiedSampler(int xstart, int xend, int ystart, int yend, int xs, int ys, bool jitter, float sopen, float sclose)
				: Sampler(xstart, xend, ystart, yend, xs * ys, sopen, sclose) {
					jitterSamples = jitter;
					xPos = xPixelStart;
					yPos = yPixelStart;
					xPixelSamples = xs;
					yPixelSamples = ys;
					buffer = new float[5 * xPixelSamples * yPixelSamples];
				}

			int StratifiedSampler::getMoreSamples(Sample *samples, ponos::RNG &rng) {
				if(yPos == yPixelEnd)
					return 0;
				int nSamples = xPixelSamples * yPixelSamples;
				// Generate stratified camera samples for (xPos, yPos)
				// // generate initial stratified samples into buffer memory
				float *bufp = buffer;
				float *imageSamples = bufp; bufp += 2 * nSamples;
				float *lensSamples = bufp; bufp += 2 * nSamples;
				float *timeSamples = bufp;
				generateStratifiedSample2D(imageSamples, xPixelSamples, yPixelSamples, rng, jitterSamples);
				generateStratifiedSample2D(imageSamples, xPixelSamples, yPixelSamples, rng, jitterSamples);
				generateStratifiedSample1D(imageSamples, xPixelSamples * yPixelSamples, rng, jitterSamples);
				// // shift stratified image samples to pixel coordinates
				for(int o = 0; o < 2 * xPixelSamples * yPixelSamples; o += 2) {
					imageSamples[o] += xPos;
					imageSamples[o + 1] += yPos;
				}
				// // decorrelate sample dimensions
				// TODO pg 354 shuffle(lensSamples, xPixelSamples * yPixelSamples, 2, rng);
				// TODO shuffle(timeSamples, xPixelSamples * yPixelSamples, 1, rng);
				// // initialize stratified samples with sample values
				for(int i = 0; i < nSamples; i++) {
					samples[i].imageX = imageSamples[2 * i];
					samples[i].imageY = imageSamples[2 * i + 1];
					samples[i].lensU = lensSamples[2 * i];
					samples[i].lensV = lensSamples[2 * i + 1];
					samples[i].time = ponos::lerp(timeSamples[i], shutterOpen, shutterClose);
					// generate stratified samples for integrators
					for(uint32 j = 0; j < samples[i].n1D.size(); i++)
						generateLatinHypercube(samples[i].oneD[j], samples[i].n1D[j], 1, rng);
					for(uint32 j = 0; j < samples[i].n2D.size(); i++)
						generateLatinHypercube(samples[i].twoD[j], samples[i].n2D[j], 2, rng);
				}
				// Advance to next pixel for stratified sampling
				if(++xPos == xPixelSamples) {
					xPos = xPixelStart;
					++yPos;
				}
				return nSamples;
			}

			Sampler* StratifiedSampler::getSubSampler(int num, int count) {
				int x0, x1, y0, y1;
				computeSubWindow(num, count, &x0, &x1, &y0, &y1);
				if(x0 == x1 || y0 == y1)
					return nullptr;
				return new StratifiedSampler(x0, x1, y0, y1, xPixelSamples, yPixelSamples, jitterSamples, shutterOpen, shutterClose);
			}
} // helios namespace
