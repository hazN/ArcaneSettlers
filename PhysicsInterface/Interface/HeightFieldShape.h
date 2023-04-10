#pragma once

#include "iShape.h"
#include <vector>

namespace physics
{
	class HeightFieldShape : public iShape
	{
	public:
		HeightFieldShape(unsigned int width, unsigned int depth, const std::vector<float>& heightData, float resolution = 1.0f);
		virtual ~HeightFieldShape();

		unsigned int GetWidth() const;
		unsigned int GetDepth() const;
		const std::vector<float>& GetHeightData() const;
		float GetResolution() const;

		static HeightFieldShape* Cast(iShape* shape);

	protected:
		HeightFieldShape(ShapeType shapeType)
			: iShape(shapeType) {}

	private:
		unsigned int m_Width;
		unsigned int m_Depth;
		std::vector<float> m_HeightData;
		float m_Resolution;

		HeightFieldShape(const HeightFieldShape&) : iShape(ShapeType::HeightField) {}
		HeightFieldShape& operator=(const HeightFieldShape&) {
			return *this;
		}
	};
}
