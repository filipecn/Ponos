#ifndef PONOS_GEOMETRY_NUMERIC_H
#define PONOS_GEOMETRY_NUMERIC_H

#include "common/defs.h"

#include <cmath>
#include <algorithm>

namespace ponos {

#ifndef INFINITY
#define INFINITY FLT_MAX
#endif

#define PI 3.14159265358979323846f
#define PI_2 6.28318530718
#define INV_PI 0.31830988618379067154f
#define INV_TWOPI 0.15915494309189533577f
#define INV_FOURPI 0.07957747154594766788f

#define SQR(A) \
		((A) * (A))
		
#define TO_DEGREES(A) \
		((A) * 180.f / PI)

#define TO_RADIANS(A) \
		((A) * PI / 180.f)
		
#define IS_ZERO(A) \
		(fabs(A) < 1e-8)
		
#define IS_EQUAL(A, B) \
		(fabs((A) - (B)) < 1e-6)
		/* round
		 * @f **[in]**
		 * @return ceil of **f**
		 */
		inline int ceil2Int(float f) {
			return static_cast<int>(f + 0.5f);
		}
		/* round
		 * @f **[in]**
		 * @return floor of **f**
		 */
		inline int floor2Int(float f) {
			return static_cast<int>(f);
		}
		/* round
		 * @f **[in]**
		 * @return next integer greater or equal to **f**
		 */
		inline int round2Int(float f) {
			return f + .5f;
		}
		/* modulus
		 * @a **[in]**
		 * @b **[in]**
		 * @return the remainder of a / b
		 */
		inline float mod(int a, int b) {
			int n = a / b;
			a -= n * b;
			if(a < 0) a += b;
			return a;
		}
		/* interpolation
		 * @t **[in]** parametric coordinate
		 * @a **[in]** lower bound
		 * @b **[in]** upper bound
		 * @return linear interpolation between **a** and **b** at **t**.
		 */
		inline float lerp(float t, float a, float b) {
			return (1.f - t) * a + t * b;
		}
		/* clamp
		 * @n **[in]** value
		 * @l **[in]** low
		 * @u **[in]** high
		 * @return clame **b** to be in **[l, h]**
		 */
		template <typename T>
		T clamp(const T& n, const T& l, const T& u) {
			return std::max(l, std::min(n, u));
		}
		/* log
		 * @x **[in]** value
		 * @return base-2 logarithm of **x**
		 */
		inline float log2(float x) {
			static float invLog2 = 1.f / logf(2.f);
			return logf(x) * invLog2;
		}
		/* log
		 * @x **[in]** value
		 * @return integer base-2 logarithm of **x**
		 */
		inline int log2Int(float x) {
			return floor2Int(log2(x));
		}
		/* query
		 * @v **[in]** value
		 * @return **true** if **v** is power of 2
		 */
		inline bool isPowerOf2(int v) {
			return (v & (v - 1)) == 0;
		}
		/* round
		 * @v **[in]** value
		 * @return the next number in power of 2
		 */
		inline uint32 roundUpPow2(uint32 v) {
			v--;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			return v + 1;
		}

		template <typename T>
			void swap(T &a, T &b) {
				T tmp = b;
				b = a;
				a = tmp;
			}
		
			inline bool solve_quadratic(float A, float B, float C, float &t0, float &t1) {
				float delta = B * B - 4.f * A * C;
					if(IS_ZERO(A) || delta <= 0.)
						return false;
							float sDelta = sqrtf(delta);
							float q;
							if(B < 0.)
								q = -0.5 * (B - sDelta);
							else q = -0.5f * (B + sDelta);
								t0 = q / A;
				t1 = C / q;
				if(t0 > t1)
					swap(t0, t1);
						return true;
			}
		
			inline float trilinear_hat_function(float r) {
				if(0.f <= r && r <= 1.f)
					return 1.f - r;
						if(-1.f <= r && r <= 0.f)
							return 1.f + r;
								return 0.f;
			}
		
			inline float quadraticBSpline(float r){
				if(-1.5f <= r && r < -0.5f)
					return 0.5f * (r + 1.5f) * (r + 1.5f);
						if(-0.5f <= r && r < 0.5f)
							return 0.75f - r * r;
								if(0.5f <= r && r < 1.5f)
									return 0.5f * (1.5f - r) * (1.5f - r);
										return 0.0f;
			}
		
			template<typename T>
			inline T bilinearInterpolation(T f00, T f10, T f11, T f01, T x, T y){
				return f00 * (1.0 - x) * (1.0 - y) + f10*x*(1.0 - y) + f01*(1.0 - x)*y + f11*x*y;
			}
		
			template<typename T>
			inline T cubicInterpolate(T p[4], T x) {
				return p[1] + 0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
			}
		
			template<typename T>
			inline T bicubicInterpolate(T p[4][4], T x, T y) {
				T arr[4];
					arr[0] = cubicInterpolate(p[0], y);
					arr[1] = cubicInterpolate(p[1], y);
					arr[2] = cubicInterpolate(p[2], y);
					arr[3] = cubicInterpolate(p[3], y);
					return cubicInterpolate(arr, x);
			}

			inline float smooth(float a, float b) {
				return std::max(0.f, 1.f - a / SQR(b));
			}
} // ponos namespace

#endif
