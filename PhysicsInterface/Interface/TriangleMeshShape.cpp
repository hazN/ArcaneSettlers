#include "TriangleMeshShape.h"

namespace physics
{
	TriangleMeshShape::TriangleMeshShape(const std::vector<Vector3>& vertices, const std::vector<unsigned int>& indices)
		: iShape(ShapeType::TriangleMesh)
		, m_Vertices(vertices)
		, m_Indices(indices)
	{ }

	TriangleMeshShape::~TriangleMeshShape()
	{ }

	const std::vector<Vector3>& TriangleMeshShape::GetVertices() const
	{
		return m_Vertices;
	}

	const std::vector<unsigned int>& TriangleMeshShape::GetIndices() const
	{
		return m_Indices;
	}

	TriangleMeshShape* TriangleMeshShape::Cast(iShape* shape)
	{
		return dynamic_cast<TriangleMeshShape*>(shape);
	}
}
