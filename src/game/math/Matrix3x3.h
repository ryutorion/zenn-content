#ifndef MATRIX3X3_H_INCLUDED
#define MATRIX3X3_H_INCLUDED

#include "Matrix2x2.h"

class Matrix3x3
{
public:
	constexpr Matrix3x3(
		float m00, float m01, float m02,
		float m10, float m11, float m12,
		float m20, float m21, float m22
	) noexcept
		: mV {
			m00, m01, m02,
			m10, m11, m12,
			m20, m21, m22
		}
	{}

	constexpr const Matrix3x3 operator*(const Matrix3x3 & m) const noexcept
	{
		return Matrix3x3(
			mV[0][0] * m.mV[0][0] + mV[0][1] * m.mV[1][0] + mV[0][2] * m.mV[2][0],
			mV[0][0] * m.mV[0][1] + mV[0][1] * m.mV[1][1] + mV[0][2] * m.mV[2][1],
			mV[0][0] * m.mV[0][2] + mV[0][1] * m.mV[1][2] + mV[0][2] * m.mV[2][2],

			mV[1][0] * m.mV[0][0] + mV[1][1] * m.mV[1][0] + mV[1][2] * m.mV[2][0],
			mV[1][0] * m.mV[0][1] + mV[1][1] * m.mV[1][1] + mV[1][2] * m.mV[2][1],
			mV[1][0] * m.mV[0][2] + mV[1][1] * m.mV[1][2] + mV[1][2] * m.mV[2][2],

			mV[2][0] * m.mV[0][0] + mV[2][1] * m.mV[1][0] + mV[2][2] * m.mV[2][0],
			mV[2][0] * m.mV[0][1] + mV[2][1] * m.mV[1][1] + mV[2][2] * m.mV[2][1],
			mV[2][0] * m.mV[0][2] + mV[2][1] * m.mV[1][2] + mV[2][2] * m.mV[2][2]
		);
	}

	constexpr float determinant() const noexcept
	{
		float c00 = mV[0][0] * Matrix2x2(
			mV[1][1], mV[1][2],
			mV[2][1], mV[2][2]
		).determinant();

		float c01 = -mV[0][1] * Matrix2x2(
			mV[1][0], mV[1][2],
			mV[2][0], mV[2][2]
		).determinant();

		float c02 = mV[0][2] * Matrix2x2(
			mV[1][0], mV[1][1],
			mV[2][0], mV[2][1]
		).determinant();

		return c00 + c01 + c02;
	}

	const Matrix3x3 inverse() const
	{
		float c00 = Matrix2x2(
			mV[1][1], mV[1][2],
			mV[2][1], mV[2][2]
		).determinant();
		float c01 = -(Matrix2x2(
			mV[1][0], mV[1][2],
			mV[2][0], mV[2][2]
		).determinant());
		float c02 = mV[0][2] * Matrix2x2(
			mV[1][0], mV[1][1],
			mV[2][0], mV[2][1]
		).determinant();

		float c10 = -(Matrix2x2(
			mV[0][1], mV[0][2],
			mV[2][1], mV[2][2]
		).determinant());
		float c11 = mV[1][1] * Matrix2x2(
			mV[0][0], mV[0][2],
			mV[2][0], mV[2][2]
		).determinant();
		float c12 = -(Matrix2x2(
			mV[0][0], mV[0][1],
			mV[2][0], mV[2][1]
		).determinant());

		float c20 = Matrix2x2(
			mV[0][1], mV[0][2],
			mV[1][1], mV[1][2]
		).determinant();
		float c21 = -(Matrix2x2(
			mV[0][0], mV[0][2],
			mV[1][0], mV[1][2]
		).determinant());
		float c22 = Matrix2x2(
			mV[0][0], mV[0][1],
			mV[1][0], mV[1][1]
		).determinant();

		float rd = 1.0f / (c00 + c01 + c02);

		return Matrix3x3(
			c00 * rd, c10 * rd, c20 * rd,
			c01 * rd, c11 * rd, c21 * rd,
			c02 * rd, c12 * rd, c22 * rd
		);
	}
private:
	float mV[3][3];
};

#endif // MATRIX3X3_H_INCLUDED
