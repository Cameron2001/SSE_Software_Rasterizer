#pragma once
#include <vector>
#include <memory>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Framebuffer.h"
#include "Camera.h"
#include "Model.h"
#include "Mesh.h"

struct alignas(16) TriangleData
{
	// screen-space bounds
	int minX, maxX, minY, maxY;

	// barycentric calculation data
	__m128 invArea;
	__m128 edgeA[3];
	__m128 edgeB[3];
	__m128 edgeC[3];
	__m128 edgeDeltaX[3]; // for pixel stepping

	//attributes
	__m128 depth[3];
	__m128 invW[3];
	__m128 u[3], v[3];
	__m128 normalX[3];
	__m128 normalY[3];
	__m128 normalZ[3];
};

class Renderer
{
public:
	Renderer();

	void renderModel(Framebuffer& framebuffer, const Camera& camera, const Model& model);
	void renderMesh(Framebuffer& framebuffer, const Camera& camera, const Mesh& mesh,
	                const glm::mat4& modelMatrix);

private:
	static constexpr int TILE_WIDTH = 16;
	static constexpr int TILE_HEIGHT = 16;
	static constexpr int TILE_SHIFT = 4;

	int mTileCountX = 0;
	int mTileCountY = 0;
	size_t mTriangleCount = 0;

	std::vector<TriangleData> mTriangleData;
	std::vector<size_t> mValidTriangles;

	std::vector<int> mBinTriangleCounts;
	std::vector<int> mBinTriangleOffsets;
	std::vector<size_t> mBinnedTriangles;
	std::vector<std::array<int, 4>> mTileRanges;
	std::vector<int> mBinWritePos;

	// lighting parameters
	__m128 lightDirX = _mm_set1_ps(0.5f);
	__m128 lightDirY = _mm_set1_ps(0.5f);
	__m128 lightDirZ = _mm_set1_ps(0.5f);
	__m128 ambientIntensity = _mm_set1_ps(0.2f);

	void preallocateBuffers(size_t vertexCount);

	void processVerticesAndAssembleTriangles(const VertexArray& vertices, const glm::mat4& mvp,
	                                         const glm::mat3& normalMatrix,
	                                         int fbWidth, int fbHeight);

	void setupTriangle(TriangleData& triangle, int* screenX, int* screenY,
	                   const float* ndcZ, const float* invW, const VertexArray& vertices,
	                   size_t baseVertex, const glm::mat3& normalMatrix);

	void binTriangles();

	void rasterizeTiles(Framebuffer& framebuffer, const Material* material);

	void rasterizeTile(Framebuffer& framebuffer, const Material* material,
	                   int tileMinX, int tileMinY, int tileMaxX, int tileMaxY,
	                   const size_t* triangleIndices, int triangleCount) const;

	void rasterizeScanline(Framebuffer& framebuffer, const Material* material,
	                       const TriangleData& triangle, int y, int startX, int endX) const;

	void fragmentShader(__m128 u, __m128 v, __m128 normalX, __m128 normalY, __m128 normalZ,
	                    const Material* material, __m128i& colors) const;

	// SIMD constants
	static inline const __m128 INV_255 = _mm_set1_ps(1.0f / 255.0f);
	static inline const __m128 MUL_255 = _mm_set1_ps(255.0f);
	static inline const __m128i MASK_FF = _mm_set1_epi32(0xFF);
	static inline const __m128i OFFSETS_I = _mm_set_epi32(3, 2, 1, 0);
	static inline const __m128i INC_XI = _mm_set1_epi32(4);
	static inline const __m128 ONE = _mm_set1_ps(1.0f);
	static inline const __m128 NEG_ONE = _mm_set1_ps(-1.0f);
	static inline const __m128 ZERO = _mm_setzero_ps();
	static inline const __m128 PIXEL_OFFSETS = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);
};
