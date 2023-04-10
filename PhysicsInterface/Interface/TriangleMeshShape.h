#pragma once

#include "iShape.h"
#include "Math.h"
#include <vector>

namespace physics
{
	class TriangleMeshShape : public iShape
	{
	public:
		TriangleMeshShape(const std::vector<Vector3>& vertices, const std::vector<unsigned int>& indices);
		virtual ~TriangleMeshShape();

		const std::vector<Vector3>& GetVertices() const;
		const std::vector<unsigned int>& GetIndices() const;

		static TriangleMeshShape* Cast(iShape* shape);

	protected:
		TriangleMeshShape(ShapeType shapeType)
			: iShape(shapeType) {}

	private:
		std::vector<Vector3> m_Vertices;
		std::vector<unsigned int> m_Indices;

		TriangleMeshShape(const TriangleMeshShape&) : iShape(ShapeType::TriangleMesh) {}
		TriangleMeshShape& operator=(const TriangleMeshShape&) {
			return *this;
		}
	};
}
