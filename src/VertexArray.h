#pragma once
#include <vector>
#include <stdexcept>

struct VertexArray
{
	std::vector<float> positionsX;
	std::vector<float> positionsY;
	std::vector<float> positionsZ;

	std::vector<float> uvsU;
	std::vector<float> uvsV;

	std::vector<float> normalsX;
	std::vector<float> normalsY;
	std::vector<float> normalsZ;

	// utility methods
	void resize(const size_t size)
	{
		if (size == 0)
		{
			throw std::invalid_argument("VertexArray size cannot be zero");
		}
		positionsX.resize(size);
		positionsY.resize(size);
		positionsZ.resize(size);

		uvsU.resize(size);
		uvsV.resize(size);

		normalsX.resize(size);
		normalsY.resize(size);
		normalsZ.resize(size);
	}

	void reserve(const size_t size)
	{
		if (size == 0)
		{
			throw std::invalid_argument("VertexArray reserve size cannot be zero");
		}
		positionsX.reserve(size);
		positionsY.reserve(size);
		positionsZ.reserve(size);

		uvsU.reserve(size);
		uvsV.reserve(size);

		normalsX.reserve(size);
		normalsY.reserve(size);
		normalsZ.reserve(size);
	}

	void clear()
	{
		positionsX.clear();
		positionsY.clear();
		positionsZ.clear();

		uvsU.clear();
		uvsV.clear();

		normalsX.clear();
		normalsY.clear();
		normalsZ.clear();
	}

	size_t size() const
	{
		return positionsX.size();
	}
};
