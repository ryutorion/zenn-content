#ifndef MATRIX2X2_H_INCLUDED
#define MATRIX2X2_H_INCLUDED

class Matrix2x2
{
public:
	Matrix2x2() noexcept = default;
    constexpr Matrix2x2(
        float m00, float m01,
        float m10, float m11
    ) noexcept
        : mV {
            m00, m01,
            m10, m11
        }
    {}

	constexpr const Matrix2x2 operator*(const Matrix2x2 & m)
	{
		return Matrix2x2(
			mV[0][0] * m.mV[0][0] + mV[0][1] * m.mV[1][0],
			mV[0][0] * m.mV[0][1] + mV[0][1] * m.mV[1][1],

			mV[1][0] * m.mV[0][0] + mV[1][1] * m.mV[1][0],
			mV[1][0] * m.mV[0][1] + mV[1][1] * m.mV[1][1]
		);
	}

    constexpr float determinant() const noexcept
    {
        return mV[0][0] * mV[1][1] - mV[0][1] * mV[1][0];
    }

	constexpr const Matrix2x2 inverse() const
	{
		float rd = 1.0f / determinant();

		return Matrix2x2(
			mV[1][1] * rd, -mV[0][1] * rd,
			-mV[1][0] * rd, mV[0][0] * rd
		);
	}

private:
    float mV[2][2];
};

#endif // MATRIX2X2_H_INCLUDED
