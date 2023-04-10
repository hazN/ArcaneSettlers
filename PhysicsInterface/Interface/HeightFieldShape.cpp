#include "HeightFieldShape.h"

namespace physics
{
	HeightFieldShape::HeightFieldShape(unsigned int width, unsigned int depth, const std::vector<float>& heightData, float resolution)
		: iShape(ShapeType::HeightField)
		, m_Width(width)
		, m_Depth(depth)
		, m_HeightData(heightData)
		, m_Resolution(resolution)
	{ }

	HeightFieldShape::~HeightFieldShape()
	{ }

	unsigned int HeightFieldShape::GetWidth() const
	{
		return m_Width;
	}

	unsigned int HeightFieldShape::GetDepth() const
	{
		return m_Depth;
	}

	const std::vector<float>& HeightFieldShape::GetHeightData() const
	{
		return m_HeightData;
	}

	float HeightFieldShape::GetResolution() const
	{
		return m_Resolution;
	}

	HeightFieldShape* HeightFieldShape::Cast(iShape* shape)
	{
		return dynamic_cast<HeightFieldShape*>(shape);
	}
}
