#define maxSteps 500
#define tstep 0.01f


// intersect ray with a box
// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
int intersectBox(float4 r_o, float4 r_d, float4 boxmin, float4 boxmax, float *tnear, float *tfar)
{
    // compute intersection of ray with all six bbox planes
    float4 invR = (float4)(1.0f,1.0f,1.0f,1.0f) / r_d;
    float4 tbot = invR * (boxmin - r_o);
    float4 ttop = invR * (boxmax - r_o);

    // re-order intersections to find smallest and largest on each axis
    float4 tmin = min(ttop, tbot);
    float4 tmax = max(ttop, tbot);

    // find the largest tmin and the smallest tmax
    float largest_tmin = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
    float smallest_tmax = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));

	*tnear = largest_tmin;
	*tfar = smallest_tmax;

	return smallest_tmax > largest_tmin;
}


uint rgbaFloatToInt(float4 rgba)
{
    rgba.x = clamp(rgba.x,0.0f,1.0f);  
    rgba.y = clamp(rgba.y,0.0f,1.0f);  
    rgba.z = clamp(rgba.z,0.0f,1.0f);  
    rgba.w = clamp(rgba.w,0.0f,1.0f);  
    return ((uint)(rgba.w*255.0f)<<24) | ((uint)(rgba.z*255.0f)<<16) | ((uint)(rgba.y*255.0f)<<8) | (uint)(rgba.x*255.0f);
}

__kernel void d_render(__global uint *d_output, uint imageW, uint imageH,
					   __global int* volume,
					   const int volumeW, const int volumeH, const int volumeD
					   )
{	
    uint x = get_global_id(0);
    uint y = get_global_id(1);
	float4 color = (float4)(1.0f,0.0f,0.0f, 1.0f);
	float4 color2 = (float4)(0.0f,0.0f,1.0f, 1.0f);

	/* to do: make it with a 3d image */
    if ((x < imageW) && (y < imageH)) {
		uint i =(y * imageW) + x;
		if(x>imageW/2 && y>imageH/2)
			d_output[i] = rgbaFloatToInt(color);
		else
			d_output[i] = rgbaFloatToInt(color2);
    }
}


// -> each thread runs over one simplex, or
// -> each thread runs over one pixel
__kernel void fill_volume(__global bool* const volume,
						  __global const float* const simplices,
						  __global float* const constraints,
						  const int volumeW, const int volumeH, const int volumeD,
						  const int numSimplices)
{
	uint idx = get_global_id(0);

	if (idx < numSimplices) {
		const int s_base = (N+1)*(K+1)*idx;
		const int c_counter = 16;
		const int c_base = (N+1)*C_PER_SIMPLEX*idx;

		SMatrix echelon, points;
		// Load matrix into registers
		for (int i = 0; i < (K+1)*(N+1); i++) {
			const int ln = i/(K+1);
			const int cl = i%(K+1);
			points[ln][cl] = simplices[idx*(K+1) + ln*numSimplices*(K+1) + cl];
		}


		float minCoord[] = {9999, 9999, 9999};
		float maxCoord[] = {-9999, -9999, -9999};
		for(int coord=0; coord<N; coord++)
		{
			for(int p=0; p<N; p++)
			{
				minCoord[coord] = min(minCoord[coord], points[coord][p]);
				maxCoord[coord] = max(maxCoord[coord], points[coord][p]);
			}
			// get the floor of the min value and the ceil of the max
			minCoord[coord] = (int) minCoord[coord];
			maxCoord[coord] = (int) (maxCoord[coord] + 1);
		}

		int vX, vY, vZ;
		for(vX=(int)minCoord[0]; vX<=(int)maxCoord[0]; ++vX)
			for(vY=(int)minCoord[1]; vY<=(int)maxCoord[1]; ++vY)
				for(vZ=(int)minCoord[2]; vZ<=(int)maxCoord[2]; ++vZ)
				{
					float discreteP[] = {vX, vY, vZ};

					bool raster = true;
					for(int i=0; i < c_counter/(N+1); i++)
					{
						float soma = 0;
						for(int coord=0; coord<N; coord++) {
							soma += discreteP[coord] * constraints[c_base + i*(N+1) + coord];
						}

						if(!(soma <= -constraints[c_base + i*(N+1) + N])) {
							raster = false;
							break;
						}
					}
					
					if(raster && vX<volumeW && vY<volumeH && vZ<volumeD && vX>=0 && vY>=0 && vZ>=0) {
						volume[vX*volumeH*volumeD + vY*volumeD + vZ] = 1;
					}
				}
	}

	/*
	if(x<volumeW && y<volumeH && z<volumeD)
	{
		volume[x*volumeH*volumeD + y*volumeD + z] = 0;
		bool raster = true;
		for(int i=0; i<numConstraints; i++)
		{
			int c_base = (N+1)*C_PER_SIMPLEX*i;
			float sum = 0;
			for(int coord=0; coord<N; coord++)
				sum += constraints[c_base + coord];

			if(!(sum <= constraints[c_base + N])) {
				raster = false;
				break;
			}
		}

		if(raster)
			volume[x*volumeH*volumeD + y*volumeD + z] = 1;
		
	} 
	*/
}

