#include "Renderer.h"
#include "Camera.h"
#include <algorithm>
#include <emmintrin.h>
#include <execution>
#include <numeric>

Renderer::Renderer()
{
	preallocateBuffers(1024);
}

void Renderer::preallocateBuffers(const size_t vertexCount)
{
	const size_t triangleCount = vertexCount / 3;
	mTriangleData.reserve(triangleCount);
	mValidTriangles.reserve(triangleCount);
	mTriangleCount = 0;
}

void Renderer::renderModel(Framebuffer& framebuffer, const Camera& camera, const Model& model)
{
	const glm::mat4& modelMatrix = model.getModelMatrix();

	for (const auto& mesh : model.getMeshes())
	{
		renderMesh(framebuffer, camera, mesh, modelMatrix);
	}
}

void Renderer::renderMesh(Framebuffer& framebuffer, const Camera& camera, const Mesh& mesh,
						  const glm::mat4& modelMatrix)
{
	const auto& vertices = mesh.getVertexArray();
	const auto material = mesh.getMaterial();

	const glm::mat4 mvp = camera.getViewProjectionMatrix() * modelMatrix * mesh.getLocalMatrix();
	const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(modelMatrix * mesh.getLocalMatrix()));

	mTriangleData.clear();
	mValidTriangles.clear();
	mTriangleCount = 0;

	preallocateBuffers(vertices.positionsX.size());

	processVerticesAndAssembleTriangles(vertices, mvp, normalMatrix, framebuffer.getWidth(), framebuffer.getHeight());

	// skip if no triangles are visible
	if (mValidTriangles.empty())
	{
		return;
	}

	rasterizeTiles(framebuffer, material);
}

void Renderer::processVerticesAndAssembleTriangles(const VertexArray& vertices, const glm::mat4& mvp,
												   const glm::mat3& normalMatrix,
												   const int fbWidth, const int fbHeight)
{
	const size_t vertexCount = vertices.positionsX.size();
	const int screenWidth = fbWidth;
	const int screenHeight = fbHeight;
	for (size_t baseVertex = 0; baseVertex + 2 < vertexCount; baseVertex += 3)
	{
		float invW[3], ndcX[3], ndcY[3], ndcZ[3];
		int screenX[3], screenY[3];
		bool isCulled = false;

		for (int i = 0; i < 3; ++i)
		{
			const size_t vertexIndex = baseVertex + i;

			// apply MVP
			const glm::vec4 clipPos = mvp * glm::vec4(
				vertices.positionsX[vertexIndex],
				vertices.positionsY[vertexIndex],
				vertices.positionsZ[vertexIndex],
				1.0f
			);

			// cull behind camera
			if (clipPos.w <= 0.0f)
			{
				isCulled = true;
				break;
			}

			// perspective division
			invW[i] = 1.0f / clipPos.w;

			ndcX[i] = clipPos.x * invW[i];
			ndcY[i] = clipPos.y * invW[i];
			ndcZ[i] = clipPos.z * invW[i];

			// from NDC [-1,1] to screen coordinates (y gets flipped)
			screenX[i] = static_cast<int>((ndcX[i] + 1.0f) * 0.5f * fbWidth);
			screenY[i] = static_cast<int>((1.0f - ndcY[i]) * 0.5f * fbHeight);
		}

		if (isCulled) continue;

		// Backface culling using signed area
		const float signedArea = static_cast<float>(screenX[1] - screenX[0]) * static_cast<float>(screenY[2] - screenY[
				0]) -
			static_cast<float>(screenX[2] - screenX[0]) * static_cast<float>(screenY[1] - screenY[0]);

		if (signedArea >= 0.0f) continue;

		mTriangleData.emplace_back();
		size_t triangleIndex = mTriangleData.size() - 1;
		TriangleData& triangle = mTriangleData[triangleIndex];

		// inverse area for barycentric coordinates and avoid division by zero
		const float invArea = (std::abs(signedArea) > 1e-6f) ? (1.0f / std::abs(signedArea)) : 0.0f;
		triangle.invArea = _mm_set1_ps(invArea);

		setupTriangle(triangle, screenX, screenY, ndcZ, invW, vertices, baseVertex, normalMatrix);

		triangle.minX = std::max(0, triangle.minX);
		triangle.maxX = std::min(screenWidth - 1, triangle.maxX);
		triangle.minY = std::max(0, triangle.minY);
		triangle.maxY = std::min(screenHeight - 1, triangle.maxY);

		mValidTriangles.push_back(triangleIndex);
		++mTriangleCount;
	}
}

void Renderer::setupTriangle(TriangleData& triangle, int* screenX, int* screenY,
							 const float* ndcZ, const float* invW, const VertexArray& vertices,
							 const size_t baseVertex, const glm::mat3& normalMatrix)
{
	// calculate bounds for binning
	triangle.minX = std::min({screenX[0], screenX[1], screenX[2]});
	triangle.maxX = std::max({screenX[0], screenX[1], screenX[2]});
	triangle.minY = std::min({screenY[0], screenY[1], screenY[2]});
	triangle.maxY = std::max({screenY[0], screenY[1], screenY[2]});

	// edge equations: Ax + By + C = 0
	const float edge1A = static_cast<float>(screenY[1] - screenY[2]);
	const float edge1B = static_cast<float>(screenX[2] - screenX[1]);
	const float edge1C = static_cast<float>(screenX[1] * screenY[2] - screenX[2] * screenY[1]);

	const float edge2A = static_cast<float>(screenY[2] - screenY[0]);
	const float edge2B = static_cast<float>(screenX[0] - screenX[2]);
	const float edge2C = static_cast<float>(screenX[2] * screenY[0] - screenX[0] * screenY[2]);

	const float edge3A = static_cast<float>(screenY[0] - screenY[1]);
	const float edge3B = static_cast<float>(screenX[1] - screenX[0]);
	const float edge3C = static_cast<float>(screenX[0] * screenY[1] - screenX[1] * screenY[0]);

	triangle.edgeA[0] = _mm_set1_ps(edge1A);
	triangle.edgeB[0] = _mm_set1_ps(edge1B);
	triangle.edgeC[0] = _mm_set1_ps(edge1C);
	triangle.edgeA[1] = _mm_set1_ps(edge2A);
	triangle.edgeB[1] = _mm_set1_ps(edge2B);
	triangle.edgeC[1] = _mm_set1_ps(edge2C);
	triangle.edgeA[2] = _mm_set1_ps(edge3A);
	triangle.edgeB[2] = _mm_set1_ps(edge3B);
	triangle.edgeC[2] = _mm_set1_ps(edge3C);

	triangle.edgeDeltaX[0] = _mm_set1_ps(edge1A * 4.0f);
	triangle.edgeDeltaX[1] = _mm_set1_ps(edge2A * 4.0f);
	triangle.edgeDeltaX[2] = _mm_set1_ps(edge3A * 4.0f);

	// Store vertex attributes
	for (int i = 0; i < 3; ++i)
	{
		const size_t vertexIndex = baseVertex + i;

		triangle.depth[i] = _mm_set1_ps(ndcZ[i]);
		triangle.invW[i] = _mm_set1_ps(invW[i]);

		triangle.u[i] = _mm_set1_ps(vertices.uvsU[vertexIndex]);
		triangle.v[i] = _mm_set1_ps(vertices.uvsV[vertexIndex]);

		//  to world space for lighting
		const glm::vec3 normal(
			vertices.normalsX[vertexIndex],
			vertices.normalsY[vertexIndex],
			vertices.normalsZ[vertexIndex]
		);
		const glm::vec3 worldNormal = glm::normalize(normalMatrix * normal);

		triangle.normalX[i] = _mm_set1_ps(worldNormal.x);
		triangle.normalY[i] = _mm_set1_ps(worldNormal.y);
		triangle.normalZ[i] = _mm_set1_ps(worldNormal.z);
	}
}

void Renderer::binTriangles()
{
	const size_t triCount = mValidTriangles.size();
	const size_t tileCount = static_cast<size_t>(mTileCountX) * mTileCountY;

	mTileRanges.resize(triCount);

	mBinTriangleCounts.assign(tileCount, 0);
	mBinTriangleOffsets.assign(tileCount + 1, 0);

	for (size_t i = 0; i < triCount; ++i)
	{
		const size_t triangleIndex = mValidTriangles[i];
		const TriangleData& tri = mTriangleData[triangleIndex];

		// tile range using triangle bounds
		const int minTX = std::clamp(tri.minX >> TILE_SHIFT, 0, mTileCountX - 1);
		const int maxTX = std::clamp(tri.maxX >> TILE_SHIFT, 0, mTileCountX - 1);
		const int minTY = std::clamp(tri.minY >> TILE_SHIFT, 0, mTileCountY - 1);
		const int maxTY = std::clamp(tri.maxY >> TILE_SHIFT, 0, mTileCountY - 1);
		mTileRanges[i] = {minTX, maxTX, minTY, maxTY};

		// Count triangles per bin
		for (int ty = minTY; ty <= maxTY; ++ty)
		{
			const size_t rowStart = static_cast<size_t>(ty) * mTileCountX;
			for (int tx = minTX; tx <= maxTX; ++tx)
				++mBinTriangleCounts[rowStart + tx];
		}
	}

	// prefix sums for offsets
	for (size_t t = 0; t < tileCount; ++t)
		mBinTriangleOffsets[t + 1] = mBinTriangleOffsets[t] + mBinTriangleCounts[t];

	const size_t totalRefs = mBinTriangleOffsets[tileCount];
	mBinnedTriangles.resize(totalRefs);

	mBinWritePos.resize(tileCount);
	std::copy(mBinTriangleOffsets.begin(), mBinTriangleOffsets.begin() + tileCount, mBinWritePos.begin());

	// Fill bins
	for (size_t i = 0; i < triCount; ++i)
	{
		const size_t triangleIndex = mValidTriangles[i];
		auto [minTX, maxTX, minTY, maxTY] = mTileRanges[i];

		for (int ty = minTY; ty <= maxTY; ++ty)
		{
			const size_t rowStart = static_cast<size_t>(ty) * mTileCountX;
			for (int tx = minTX; tx <= maxTX; ++tx)
			{
				const size_t binIndex = rowStart + tx;
				const int pos = mBinWritePos[binIndex]++;
				mBinnedTriangles[pos] = triangleIndex;
			}
		}
	}
}

void Renderer::rasterizeTiles(Framebuffer& framebuffer, const Material* material)
{
	const int fbWidth = framebuffer.getWidth();
	const int fbHeight = framebuffer.getHeight();

	// tile grid dimensions
	const int newTileCountX = (fbWidth + TILE_WIDTH - 1) >> TILE_SHIFT;
	const int newTileCountY = (fbHeight + TILE_HEIGHT - 1) >> TILE_SHIFT;
	const size_t totalTiles = static_cast<size_t>(newTileCountX) * newTileCountY;

	if (newTileCountX != mTileCountX || newTileCountY != mTileCountY)
	{
		mTileCountX = newTileCountX;
		mTileCountY = newTileCountY;
		mBinTriangleCounts.resize(totalTiles);
		mBinTriangleOffsets.resize(totalTiles + 1);
		mBinnedTriangles.reserve(mValidTriangles.size() * 4); // estimate for bin overlap
		mTileRanges.clear();
	}

	binTriangles();

	std::vector<size_t> tileIndices(totalTiles);
	std::ranges::iota(tileIndices, 0);

	std::for_each(std::execution::par, tileIndices.begin(), tileIndices.end(),
				  [&](const size_t tileIndex)
				  {
					  const int triangleCount = mBinTriangleCounts[tileIndex];
					  if (triangleCount == 0)
						  return;

					  const int tileX = static_cast<int>(tileIndex % mTileCountX);
					  const int tileY = static_cast<int>(tileIndex / mTileCountX);

					  const int tileMinX = tileX << TILE_SHIFT;
					  const int tileMinY = tileY << TILE_SHIFT;
					  const int tileMaxX = std::min(tileMinX + TILE_WIDTH, fbWidth);
					  const int tileMaxY = std::min(tileMinY + TILE_HEIGHT, fbHeight);

					  const size_t offset = mBinTriangleOffsets[tileIndex];
					  const size_t* triangleIndices = &mBinnedTriangles[offset];

					  rasterizeTile(framebuffer, material,
									tileMinX, tileMinY, tileMaxX, tileMaxY,
									triangleIndices, triangleCount);
				  });
}

void Renderer::rasterizeScanline(Framebuffer& framebuffer, const Material* material,
								 const TriangleData& triangle, int y, int startX, int endX) const
{
	const __m128 localZero = ZERO;
	const __m128 localOne = ONE;
	const __m128 localNegOne = NEG_ONE;
	const __m128i localIncXi = INC_XI;

	__m128 yFloat = _mm_set1_ps(static_cast<float>(y));
	__m128i yInt = _mm_set1_epi32(y);

	int baseX = startX;
	int quadCount = ((endX - baseX) + 3) >> 2; // ceiling division by 4

	// process 4 pixels at once 
	__m128 xBase = _mm_set_ps(baseX + 3.0f, baseX + 2.0f, baseX + 1.0f, baseX + 0.0f);
	__m128i xInt = _mm_add_epi32(_mm_set1_epi32(baseX), OFFSETS_I);

	// evaluate edge equations at y position
	__m128 bc0 = _mm_fmadd_ps(triangle.edgeB[0], yFloat, triangle.edgeC[0]);
	__m128 bc1 = _mm_fmadd_ps(triangle.edgeB[1], yFloat, triangle.edgeC[1]);
	__m128 bc2 = _mm_fmadd_ps(triangle.edgeB[2], yFloat, triangle.edgeC[2]);

	__m128 edge0 = _mm_fmadd_ps(triangle.edgeA[0], xBase, bc0);
	__m128 edge1 = _mm_fmadd_ps(triangle.edgeA[1], xBase, bc1);
	__m128 edge2 = _mm_fmadd_ps(triangle.edgeA[2], xBase, bc2);

	for (int q = 0; q < quadCount; ++q)
	{
		// check if pixels inside triangle (edge value <= 0)
		__m128 inside0 = _mm_cmple_ps(edge0, localZero);
		__m128 inside1 = _mm_cmple_ps(edge1, localZero);
		__m128 inside2 = _mm_cmple_ps(edge2, localZero);
		int insideMask = _mm_movemask_ps(_mm_and_ps(_mm_and_ps(inside0, inside1), inside2));

		if (!insideMask)
		{
			// skip
			edge0 = _mm_add_ps(edge0, triangle.edgeDeltaX[0]);
			edge1 = _mm_add_ps(edge1, triangle.edgeDeltaX[1]);
			edge2 = _mm_add_ps(edge2, triangle.edgeDeltaX[2]);
			xInt = _mm_add_epi32(xInt, localIncXi);
			continue;
		}

		// barycentric coords
		__m128 negInvArea = _mm_mul_ps(localNegOne, triangle.invArea);
		__m128 w0 = _mm_mul_ps(edge0, negInvArea);
		__m128 w1 = _mm_mul_ps(edge1, negInvArea);
		__m128 w2 = _mm_sub_ps(_mm_sub_ps(localOne, w0), w1);

		// interpolate depth
		__m128 depth = _mm_fmadd_ps(w2, triangle.depth[2],
									_mm_fmadd_ps(w1, triangle.depth[1],
												 _mm_mul_ps(w0, triangle.depth[0])));

		int depthPassMask = framebuffer.depthTest(xInt, yInt, depth);
		insideMask &= depthPassMask;

		if (!insideMask)
		{
			edge0 = _mm_add_ps(edge0, triangle.edgeDeltaX[0]);
			edge1 = _mm_add_ps(edge1, triangle.edgeDeltaX[1]);
			edge2 = _mm_add_ps(edge2, triangle.edgeDeltaX[2]);
			xInt = _mm_add_epi32(xInt, localIncXi);
			continue;
		}

		// perspective correction for texture coords
		__m128 p0 = _mm_mul_ps(w0, triangle.invW[0]);
		__m128 p1 = _mm_mul_ps(w1, triangle.invW[1]);
		__m128 p2 = _mm_mul_ps(w2, triangle.invW[2]);
		__m128 invSum = _mm_add_ps(_mm_add_ps(p0, p1), p2);

		__m128 rcp = _mm_div_ps(ONE, invSum);


		// correct weights with perspective divide
		p0 = _mm_mul_ps(p0, rcp);
		p1 = _mm_mul_ps(p1, rcp);
		p2 = _mm_mul_ps(p2, rcp);

		// interpolate attributes
		__m128 texU = _mm_fmadd_ps(p2, triangle.u[2],
								   _mm_fmadd_ps(p1, triangle.u[1],
												_mm_mul_ps(p0, triangle.u[0])));

		__m128 texV = _mm_fmadd_ps(p2, triangle.v[2],
								   _mm_fmadd_ps(p1, triangle.v[1],
												_mm_mul_ps(p0, triangle.v[0])));

		__m128 normalX = _mm_fmadd_ps(p2, triangle.normalX[2],
									  _mm_fmadd_ps(p1, triangle.normalX[1],
												   _mm_mul_ps(p0, triangle.normalX[0])));

		__m128 normalY = _mm_fmadd_ps(p2, triangle.normalY[2],
									  _mm_fmadd_ps(p1, triangle.normalY[1],
												   _mm_mul_ps(p0, triangle.normalY[0])));

		__m128 normalZ = _mm_fmadd_ps(p2, triangle.normalZ[2],
									  _mm_fmadd_ps(p1, triangle.normalZ[1],
												   _mm_mul_ps(p0, triangle.normalZ[0])));

		__m128i colors;
		fragmentShader(texU, texV, normalX, normalY, normalZ, material, colors);

		framebuffer.setDepth(xInt, yInt, depth, insideMask);
		framebuffer.setPixel(xInt, yInt, colors, insideMask);

		edge0 = _mm_add_ps(edge0, triangle.edgeDeltaX[0]);
		edge1 = _mm_add_ps(edge1, triangle.edgeDeltaX[1]);
		edge2 = _mm_add_ps(edge2, triangle.edgeDeltaX[2]);
		xInt = _mm_add_epi32(xInt, localIncXi);
	}
}

void Renderer::rasterizeTile(Framebuffer& framebuffer, const Material* material,
							 const int tileMinX, const int tileMinY, const int tileMaxX, const int tileMaxY,
							 const size_t* triangleIndices, const int triangleCount) const
{
	for (int i = 0; i < triangleCount; ++i)
	{
		const size_t triangleIndex = triangleIndices[i];
		if (triangleIndex >= mTriangleData.size()) continue;

		const TriangleData& triangle = mTriangleData[triangleIndex];

		const int minX = std::max(tileMinX, triangle.minX);
		const int maxX = std::min(tileMaxX - 1, triangle.maxX);
		const int minY = std::max(tileMinY, triangle.minY);
		const int maxY = std::min(tileMaxY - 1, triangle.maxY);

		if (minX > maxX || minY > maxY) continue;

		for (int y = minY; y <= maxY; ++y)
		{
			rasterizeScanline(framebuffer, material, triangle, y, minX, maxX + 1);
		}
	}
}

void Renderer::fragmentShader(__m128 u, __m128 v, __m128 normalX, __m128 normalY, __m128 normalZ,
							  const Material* material, __m128i& colors) const
{
	if (!material)
	{
		colors = _mm_set1_epi32(0x00FFFF);
		return;
	}

	// normal light dot product for diffuse lighting
	__m128 dot = _mm_add_ps(
		_mm_mul_ps(normalX, lightDirX),
		_mm_add_ps(
			_mm_mul_ps(normalY, lightDirY),
			_mm_mul_ps(normalZ, lightDirZ)
		)
	);

	// lambert term (clamp to [0,1])
	__m128 lambert = _mm_max_ps(dot, ZERO);
	lambert = _mm_min_ps(lambert, ONE);

	// add ambient light and clamp
	__m128 lighting = _mm_min_ps(_mm_add_ps(ambientIntensity, lambert), ONE);

	// get texture color or use white
	__m128i texColor;
	const Texture* diffuseMap = material ? material->getDiffuseTexture() : nullptr;
	if (diffuseMap && diffuseMap->isLoaded())
	{
		texColor = diffuseMap->sample(u, v);
	}
	else
	{
		texColor = _mm_set1_epi32(0xFFFFFF);
	}

	// split RGB channels
	__m128i r = _mm_and_si128(texColor, MASK_FF);
	__m128i g = _mm_and_si128(_mm_srli_epi32(texColor, 8), MASK_FF);
	__m128i b = _mm_and_si128(_mm_srli_epi32(texColor, 16), MASK_FF);

	// apply lighting
	__m128 rFloat = _mm_mul_ps(_mm_cvtepi32_ps(r), INV_255);
	__m128 gFloat = _mm_mul_ps(_mm_cvtepi32_ps(g), INV_255);
	__m128 bFloat = _mm_mul_ps(_mm_cvtepi32_ps(b), INV_255);

	rFloat = _mm_mul_ps(rFloat, lighting);
	gFloat = _mm_mul_ps(gFloat, lighting);
	bFloat = _mm_mul_ps(bFloat, lighting);

	rFloat = _mm_mul_ps(rFloat, MUL_255);
	gFloat = _mm_mul_ps(gFloat, MUL_255);
	bFloat = _mm_mul_ps(bFloat, MUL_255);

	__m128i rOut = _mm_cvtps_epi32(rFloat);
	__m128i gOut = _mm_cvtps_epi32(gFloat);
	__m128i bOut = _mm_cvtps_epi32(bFloat);

	// pack to output RGB format
	rOut = _mm_and_si128(rOut, MASK_FF);
	gOut = _mm_slli_epi32(_mm_and_si128(gOut, MASK_FF), 8);
	bOut = _mm_slli_epi32(_mm_and_si128(bOut, MASK_FF), 16);

	colors = _mm_or_si128(
		_mm_or_si128(rOut, gOut),
		bOut
	);
}
