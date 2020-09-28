#ifndef MATRIX4X4_H_INCLUDED
#define MATRIX4X4_H_INCLUDED

#include "Matrix3x3.h"

class Matrix4x4
{
public:
	Matrix4x4() noexcept = default;

    constexpr Matrix4x4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33
    ) noexcept
        : mV {
            m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33
        }
    {}

    Matrix4x4 & operator+=(const Matrix4x4 & m) noexcept
	{
		mV[0][0] += m.mV[0][0];
		mV[0][1] += m.mV[0][1];
		mV[0][2] += m.mV[0][2];
		mV[0][3] += m.mV[0][3];

		mV[1][0] += m.mV[1][0];
		mV[1][1] += m.mV[1][1];
		mV[1][2] += m.mV[1][2];
		mV[1][3] += m.mV[1][3];

		mV[2][0] += m.mV[2][0];
		mV[2][1] += m.mV[2][1];
		mV[2][2] += m.mV[2][2];
		mV[2][3] += m.mV[2][3];

		mV[3][0] += m.mV[3][0];
		mV[3][1] += m.mV[3][1];
		mV[3][2] += m.mV[3][2];
		mV[3][3] += m.mV[3][3];

		return *this;
	}

    constexpr const Matrix4x4 operator+(const Matrix4x4 & m) const noexcept
	{
		return Matrix4x4(
			mV[0][0] + m.mV[0][0],
			mV[0][1] + m.mV[0][1],
			mV[0][2] + m.mV[0][2],
			mV[0][3] + m.mV[0][3],

			mV[1][0] + m.mV[1][0],
			mV[1][1] + m.mV[1][1],
			mV[1][2] + m.mV[1][2],
			mV[1][3] + m.mV[1][3],

			mV[2][0] + m.mV[2][0],
			mV[2][1] + m.mV[2][1],
			mV[2][2] + m.mV[2][2],
			mV[2][3] + m.mV[2][3],

			mV[3][0] + m.mV[3][0],
			mV[3][1] + m.mV[3][1],
			mV[3][2] + m.mV[3][2],
			mV[3][3] + m.mV[3][3]
		);
	}

    Matrix4x4 & operator-=(const Matrix4x4 & m) noexcept
	{
		mV[0][0] -= m.mV[0][0];
		mV[0][1] -= m.mV[0][1];
		mV[0][2] -= m.mV[0][2];
		mV[0][3] -= m.mV[0][3];

		mV[1][0] -= m.mV[1][0];
		mV[1][1] -= m.mV[1][1];
		mV[1][2] -= m.mV[1][2];
		mV[1][3] -= m.mV[1][3];

		mV[2][0] -= m.mV[2][0];
		mV[2][1] -= m.mV[2][1];
		mV[2][2] -= m.mV[2][2];
		mV[2][3] -= m.mV[2][3];

		mV[3][0] -= m.mV[3][0];
		mV[3][1] -= m.mV[3][1];
		mV[3][2] -= m.mV[3][2];
		mV[3][3] -= m.mV[3][3];

		return *this;
	}

    constexpr const Matrix4x4 operator-(const Matrix4x4 & m) const noexcept
	{
		return Matrix4x4(
			mV[0][0] - m.mV[0][0],
			mV[0][1] - m.mV[0][1],
			mV[0][2] - m.mV[0][2],
			mV[0][3] - m.mV[0][3],

			mV[1][0] - m.mV[1][0],
			mV[1][1] - m.mV[1][1],
			mV[1][2] - m.mV[1][2],
			mV[1][3] - m.mV[1][3],

			mV[2][0] - m.mV[2][0],
			mV[2][1] - m.mV[2][1],
			mV[2][2] - m.mV[2][2],
			mV[2][3] - m.mV[2][3],

			mV[3][0] - m.mV[3][0],
			mV[3][1] - m.mV[3][1],
			mV[3][2] - m.mV[3][2],
			mV[3][3] - m.mV[3][3]
		);
	}

    Matrix4x4 & operator*=(const float s) noexcept
    {
        mV[0][0] *= s;
        mV[0][1] *= s;
        mV[0][2] *= s;
        mV[0][3] *= s;

        mV[1][0] *= s;
        mV[1][1] *= s;
        mV[1][2] *= s;
        mV[1][3] *= s;

        mV[2][0] *= s;
        mV[2][1] *= s;
        mV[2][2] *= s;
        mV[2][3] *= s;

        mV[3][0] *= s;
        mV[3][1] *= s;
        mV[3][2] *= s;
        mV[3][3] *= s;

        return *this;
    }

    constexpr const Matrix4x4 operator*(const float s) const noexcept
    {
        return Matrix4x4(
            mV[0][0] * s,
            mV[0][1] * s,
            mV[0][2] * s,
            mV[0][3] * s,
            mV[1][0] * s,
            mV[1][1] * s,
            mV[1][2] * s,
            mV[1][3] * s,
            mV[2][0] * s,
            mV[2][1] * s,
            mV[2][2] * s,
            mV[2][3] * s,
            mV[3][0] * s,
            mV[3][1] * s,
            mV[3][2] * s,
            mV[3][3] * s
        );
    }
	constexpr const Matrix4x4 operator*(const Matrix4x4 & m) const noexcept
	{
		return Matrix4x4(
			mV[0][0] * m.mV[0][0] + mV[0][1] * m.mV[1][0] + mV[0][2] * m.mV[2][0] + mV[0][3] * m.mV[3][0],
			mV[0][0] * m.mV[0][1] + mV[0][1] * m.mV[1][1] + mV[0][2] * m.mV[2][1] + mV[0][3] * m.mV[3][1],
			mV[0][0] * m.mV[0][2] + mV[0][1] * m.mV[1][2] + mV[0][2] * m.mV[2][2] + mV[0][3] * m.mV[3][2],
			mV[0][0] * m.mV[0][3] + mV[0][1] * m.mV[1][3] + mV[0][2] * m.mV[2][3] + mV[0][3] * m.mV[3][3],

			mV[1][0] * m.mV[0][0] + mV[1][1] * m.mV[1][0] + mV[1][2] * m.mV[2][0] + mV[1][3] * m.mV[3][0],
			mV[1][0] * m.mV[0][1] + mV[1][1] * m.mV[1][1] + mV[1][2] * m.mV[2][1] + mV[1][3] * m.mV[3][1],
			mV[1][0] * m.mV[0][2] + mV[1][1] * m.mV[1][2] + mV[1][2] * m.mV[2][2] + mV[1][3] * m.mV[3][2],
			mV[1][0] * m.mV[0][3] + mV[1][1] * m.mV[1][3] + mV[1][2] * m.mV[2][3] + mV[1][3] * m.mV[3][3],

			mV[2][0] * m.mV[0][0] + mV[2][1] * m.mV[1][0] + mV[2][2] * m.mV[2][0] + mV[2][3] * m.mV[3][0],
			mV[2][0] * m.mV[0][1] + mV[2][1] * m.mV[1][1] + mV[2][2] * m.mV[2][1] + mV[2][3] * m.mV[3][1],
			mV[2][0] * m.mV[0][2] + mV[2][1] * m.mV[1][2] + mV[2][2] * m.mV[2][2] + mV[2][3] * m.mV[3][2],
			mV[2][0] * m.mV[0][3] + mV[2][1] * m.mV[1][3] + mV[2][2] * m.mV[2][3] + mV[2][3] * m.mV[3][3],

			mV[3][0] * m.mV[0][0] + mV[3][1] * m.mV[1][0] + mV[3][2] * m.mV[2][0] + mV[3][3] * m.mV[3][0],
			mV[3][0] * m.mV[0][1] + mV[3][1] * m.mV[1][1] + mV[3][2] * m.mV[2][1] + mV[3][3] * m.mV[3][1],
			mV[3][0] * m.mV[0][2] + mV[3][1] * m.mV[1][2] + mV[3][2] * m.mV[2][2] + mV[3][3] * m.mV[3][2],
			mV[3][0] * m.mV[0][3] + mV[3][1] * m.mV[1][3] + mV[3][2] * m.mV[2][3] + mV[3][3] * m.mV[3][3]
		);
	}

    Matrix4x4 & operator*=(const Matrix4x4 & m) noexcept
    {
        *this = *this * m;

        return *this;
    }

    constexpr const Matrix4x4 transpose() const noexcept
    {
        return Matrix4x4(
            mV[0][0], mV[1][0], mV[2][0], mV[3][0],
            mV[0][1], mV[1][1], mV[2][1], mV[3][1],
            mV[0][2], mV[1][2], mV[2][2], mV[3][2],
            mV[0][3], mV[1][3], mV[2][3], mV[3][3]
        );
    }

    constexpr float determinant() const noexcept
	{
		float c00 = Matrix3x3(
			mV[1][1], mV[1][2], mV[1][3],
			mV[2][1], mV[2][2], mV[2][3],
			mV[3][1], mV[3][2], mV[3][3]
		).determinant();

		float c01 = -(Matrix3x3(
			mV[1][0], mV[1][2], mV[1][3],
			mV[2][0], mV[2][2], mV[2][3],
			mV[3][0], mV[3][2], mV[3][3]
		).determinant());

		float c02 = Matrix3x3(
			mV[1][0], mV[1][1], mV[1][3],
			mV[2][0], mV[2][1], mV[2][3],
			mV[3][0], mV[3][1], mV[3][3]
		).determinant();

		float c03 = -(Matrix3x3(
			mV[1][0], mV[1][1], mV[1][2],
			mV[2][0], mV[2][1], mV[2][2],
			mV[3][0], mV[3][1], mV[3][2]
		).determinant());

		return mV[0][0] * c00 + mV[0][1] * c01 + mV[0][2] * c02 + mV[0][3] * c03;
	}

	constexpr const Matrix4x4 inverse() const
	{
		float c00 = Matrix3x3(
			mV[1][1], mV[1][2], mV[1][3],
			mV[2][1], mV[2][2], mV[2][3],
			mV[3][1], mV[3][2], mV[3][3]
		).determinant();
		float c01 = -(Matrix3x3(
			mV[1][0], mV[1][2], mV[1][3],
			mV[2][0], mV[2][2], mV[2][3],
			mV[3][0], mV[3][2], mV[3][3]
		).determinant());
		float c02 = Matrix3x3(
			mV[1][0], mV[1][1], mV[1][3],
			mV[2][0], mV[2][1], mV[2][3],
			mV[3][0], mV[3][1], mV[3][3]
		).determinant();
		float c03 = -(Matrix3x3(
			mV[1][0], mV[1][1], mV[1][2],
			mV[2][0], mV[2][1], mV[2][2],
			mV[3][0], mV[3][1], mV[3][2]
		).determinant());

		float c10 = -(Matrix3x3(
			mV[0][1], mV[0][2], mV[0][3],
			mV[2][1], mV[2][2], mV[2][3],
			mV[3][1], mV[3][2], mV[3][3]
		).determinant());
		float c11 = Matrix3x3(
			mV[0][0], mV[0][2], mV[0][3],
			mV[2][0], mV[2][2], mV[2][3],
			mV[3][0], mV[3][2], mV[3][3]
		).determinant();
		float c12 = -(Matrix3x3(
			mV[0][0], mV[0][1], mV[0][3],
			mV[2][0], mV[2][1], mV[2][3],
			mV[3][0], mV[3][1], mV[3][3]
		).determinant());
		float c13 = Matrix3x3(
			mV[0][0], mV[0][1], mV[0][2],
			mV[2][0], mV[2][1], mV[2][2],
			mV[3][0], mV[3][1], mV[3][2]
		).determinant();

		float c20 = Matrix3x3(
			mV[0][1], mV[0][2], mV[0][3],
			mV[1][1], mV[1][2], mV[1][3],
			mV[3][1], mV[3][2], mV[3][3]
		).determinant();
		float c21 = -(Matrix3x3(
			mV[0][0], mV[0][2], mV[0][3],
			mV[1][0], mV[1][2], mV[1][3],
			mV[3][0], mV[3][2], mV[3][3]
		).determinant());
		float c22 = Matrix3x3(
			mV[0][0], mV[0][1], mV[0][3],
			mV[1][0], mV[1][1], mV[1][3],
			mV[3][0], mV[3][1], mV[3][3]
		).determinant();
		float c23 = -(Matrix3x3(
			mV[0][0], mV[0][1], mV[0][2],
			mV[1][0], mV[1][1], mV[1][2],
			mV[3][0], mV[3][1], mV[3][2]
		).determinant());

		float c30 = -(Matrix3x3(
			mV[0][1], mV[0][2], mV[0][3],
			mV[1][1], mV[1][2], mV[1][3],
			mV[2][1], mV[2][2], mV[2][3]
		).determinant());
		float c31 = Matrix3x3(
			mV[0][0], mV[0][2], mV[0][3],
			mV[1][0], mV[1][2], mV[1][3],
			mV[2][0], mV[2][2], mV[2][3]
		).determinant();
		float c32 = -(Matrix3x3(
			mV[0][0], mV[0][1], mV[0][3],
			mV[1][0], mV[1][1], mV[1][3],
			mV[2][0], mV[2][1], mV[2][3]
		).determinant());
		float c33 = Matrix3x3(
			mV[0][0], mV[0][1], mV[0][2],
			mV[1][0], mV[1][1], mV[1][2],
			mV[2][0], mV[2][1], mV[2][2]
		).determinant();

		float rd = 1.0f / (mV[0][0] * c00 + mV[0][1] * c01 + mV[0][2] * c02 + mV[0][3] * c03);

		return Matrix4x4(
			c00 * rd, c10 * rd, c20 * rd, c30 * rd,
			c01 * rd, c11 * rd, c21 * rd, c31 * rd,
			c02 * rd, c12 * rd, c22 * rd, c32 * rd,
			c03 * rd, c13 * rd, c23 * rd, c33 * rd
		);
	}
private:
    float mV[4][4];
};

inline constexpr const Matrix4x4 operator*(const float s, const Matrix4x4 & m) noexcept;

//==============================================================================
// Inline functions
//==============================================================================
inline constexpr const Matrix4x4 operator*(const float s, const Matrix4x4 & m) noexcept
{
    return m * s;
}

#endif // MATRIX4X4_H_INCLUDED
