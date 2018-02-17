#pragma once

#include "compute.h"
#include "fornos.h"
#include "math.h"
#include "timing.h"
#include <glad/glad.h>
#include <memory>
#include <vector>

struct MapUV;
class MeshMapping;

class BentNormalsSolver
{
public:
	struct Params
	{
		size_t sampleCount;
		float minDistance;
		float maxDistance;
		bool tangentSpace;
	};

public:
	BentNormalsSolver(const Params &params) : _params(params) {}

	void init(std::shared_ptr<const CompressedMapUV> map, std::shared_ptr<MeshMapping> meshMapping);
	bool runStep();
	Vector3* getResults();

	inline size_t width() const { return _mapWidth; }
	inline size_t height() const { return _mapHeight; }
	inline float progress() const
	{
		return (float)(_workOffset) / (float)(_workCount * _params.sampleCount);
	}

	inline std::shared_ptr<const CompressedMapUV> uvMap() const { return _uvMap; }

private:
	Params _params;
	size_t _workOffset;
	size_t _workCount;
	size_t _mapWidth;
	size_t _mapHeight;

	size_t _sampleIndex;

	struct ShaderParams
	{
		uint32_t sampleCount;
		uint32_t samplePermCount;
		float minDistance;
		float maxDistance;
	};

	struct RayData
	{
		Vector3 o; float _pad0;
		Vector3 d; float _pad1;
		Vector3 tx; float _pad2;
		Vector3 ty; float _pad3;
	};

	GLuint _rayProgram;
	GLuint _bentnormalsProgram;
	GLuint _avgProgram;
	GLuint _tanspaceProgram;
	std::unique_ptr<ComputeBuffer<ShaderParams> > _paramsCB;
	std::unique_ptr<ComputeBuffer<Vector4> > _samplesCB;
	std::unique_ptr<ComputeBuffer<RayData> > _rayDataCB;
	std::unique_ptr<ComputeBuffer<Vector4> > _resultsMiddleCB;
	std::unique_ptr<ComputeBuffer<Vector3> > _resultsFinalCB;

	std::shared_ptr<const CompressedMapUV> _uvMap;
	std::shared_ptr<MeshMapping> _meshMapping;

	Timing _timing;
};

class BentNormalsTask : public FornosTask
{
public:
	BentNormalsTask(std::unique_ptr<BentNormalsSolver> solver, const char *outputPath);
	~BentNormalsTask();

	bool runStep();
	void finish();
	float progress() const;
	const char* name() const { return "Bent normals"; }

private:
	std::unique_ptr<BentNormalsSolver> _solver;
	std::string _outputPath;
};