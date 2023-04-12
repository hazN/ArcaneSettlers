#pragma once

#include "ShapeType.h"

namespace physics
{
	class iShape
	{
	public:
		virtual ~iShape() {}

		ShapeType GetShapeType() const {
			return m_ShapeType;
		}

		void SetUserData(int id) {
			m_UserData = id;
		}

		int GetUserData() const {
			return m_UserData;
		}

	protected:
		iShape(ShapeType shapeType)
			: m_ShapeType(shapeType)
		{ }

	private:
		ShapeType m_ShapeType;
		int m_UserData; 

		iShape(const iShape&) {}
		iShape& operator=(const iShape&) {}
	};
}
