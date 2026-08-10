#ifndef NET_MINECRAFT_UTIL__MatrixStack_H__
#define NET_MINECRAFT_UTIL__MatrixStack_H__

#include "Matrix4f.h"
#include <vector>

class MatrixStack
{
public:
    MatrixStack() {
        m_stack.push_back(Matrix4f::identity());
    }

    void loadIdentity() {
        if (!m_stack.empty()) {
            m_stack.back().setIdentity();
        }
    }

    void pushMatrix() {
        if (!m_stack.empty()) {
            m_stack.push_back(m_stack.back());
        }
    }

    void popMatrix() {
        if (m_stack.size() > 1) {
            m_stack.pop_back();
        }
    }

    void translate(float x, float y, float z) {
        if (!m_stack.empty()) {
            m_stack.back().multiply(Matrix4f::createTranslation(x, y, z));
        }
    }

    void rotate(float angleDegrees, float x, float y, float z) {
        if (!m_stack.empty()) {
            m_stack.back().multiply(Matrix4f::createRotation(angleDegrees, x, y, z));
        }
    }

    void scale(float x, float y, float z) {
        if (!m_stack.empty()) {
            m_stack.back().multiply(Matrix4f::createScale(x, y, z));
        }
    }

    void ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
        if (!m_stack.empty()) {
            m_stack.back().multiply(Matrix4f::createOrtho(left, right, bottom, top, zNear, zFar));
        }
    }

    void perspective(float fovyDegrees, float aspect, float zNear, float zFar) {
        if (!m_stack.empty()) {
            m_stack.back().multiply(Matrix4f::createPerspective(fovyDegrees, aspect, zNear, zFar));
        }
    }

    const Matrix4f& getTop() const {
        return m_stack.back();
    }

    Matrix4f& getTop() {
        return m_stack.back();
    }

    void setTop(const Matrix4f& mat) {
        if (!m_stack.empty()) {
            m_stack.back() = mat;
        }
    }

private:
    std::vector<Matrix4f> m_stack;
};

#endif /* NET_MINECRAFT_UTIL__MatrixStack_H__ */
