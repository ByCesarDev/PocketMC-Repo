#ifndef NET_MINECRAFT_UTIL__Matrix4f_H__
#define NET_MINECRAFT_UTIL__Matrix4f_H__

#include <cmath>
#include <cstring>
#include "Mth.h"

class Matrix4f
{
public:
    float m[16];

    Matrix4f() {
        setIdentity();
    }

    void setIdentity() {
        std::memset(m, 0, 16 * sizeof(float));
        m[0] = 1.0f;
        m[5] = 1.0f;
        m[10] = 1.0f;
        m[15] = 1.0f;
    }

    static Matrix4f identity() {
        Matrix4f result;
        result.setIdentity();
        return result;
    }

    Matrix4f operator*(const Matrix4f& o) const {
        Matrix4f r;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                r.m[row + col * 4] = 
                    m[row + 0 * 4] * o.m[0 + col * 4] +
                    m[row + 1 * 4] * o.m[1 + col * 4] +
                    m[row + 2 * 4] * o.m[2 + col * 4] +
                    m[row + 3 * 4] * o.m[3 + col * 4];
            }
        }
        return r;
    }

    Matrix4f& multiply(const Matrix4f& o) {
        *this = *this * o;
        return *this;
    }

    static Matrix4f createTranslation(float x, float y, float z) {
        Matrix4f res;
        res.m[12] = x;
        res.m[13] = y;
        res.m[14] = z;
        return res;
    }

    static Matrix4f createScale(float x, float y, float z) {
        Matrix4f res;
        res.m[0] = x;
        res.m[5] = y;
        res.m[10] = z;
        return res;
    }

    static Matrix4f createRotation(float angleDegrees, float x, float y, float z) {
        Matrix4f res;
        float rad = angleDegrees * (Mth::PI / 180.0f);
        float c = Mth::cos(rad);
        float s = Mth::sin(rad);
        float len = std::sqrt(x * x + y * y + z * z);
        if (len > 0.00001f) {
            x /= len; y /= len; z /= len;
        }
        float nc = 1.0f - c;

        res.m[0] = x * x * nc + c;
        res.m[1] = y * x * nc + z * s;
        res.m[2] = x * z * nc - y * s;

        res.m[4] = x * y * nc - z * s;
        res.m[5] = y * y * nc + c;
        res.m[6] = y * z * nc + x * s;

        res.m[8] = x * z * nc + y * s;
        res.m[9] = y * z * nc - x * s;
        res.m[10] = z * z * nc + c;

        return res;
    }

    static Matrix4f createOrtho(float left, float right, float bottom, float top, float zNear, float zFar) {
        Matrix4f res;
        res.setIdentity();
        res.m[0] = 2.0f / (right - left);
        res.m[5] = 2.0f / (top - bottom);
        res.m[10] = -2.0f / (zFar - zNear);
        res.m[12] = -(right + left) / (right - left);
        res.m[13] = -(top + bottom) / (top - bottom);
        res.m[14] = -(zFar + zNear) / (zFar - zNear);
        return res;
    }

    static Matrix4f createPerspective(float fovyDegrees, float aspect, float zNear, float zFar) {
        Matrix4f res;
        std::memset(res.m, 0, 16 * sizeof(float));
        float rad = fovyDegrees * (Mth::PI / 180.0f);
        float tanHalfFovy = std::tan(rad / 2.0f);

        res.m[0] = 1.0f / (aspect * tanHalfFovy);
        res.m[5] = 1.0f / tanHalfFovy;
        res.m[10] = -(zFar + zNear) / (zFar - zNear);
        res.m[11] = -1.0f;
        res.m[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
        return res;
    }

    const float* getValues() const { return m; }
    float* getValues() { return m; }
};

#endif /* NET_MINECRAFT_UTIL__Matrix4f_H__ */
