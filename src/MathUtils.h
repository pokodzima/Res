#pragma once
#include "raylib.h"
#include "raymath.h"

namespace res
{
    // Column-major mapping (raylib):
    // col0: {m0,  m1,  m2 }  -> Right
    // col1: {m4,  m5,  m6 }  -> Up
    // col2: {m8,  m9,  m10}  -> Forward (-Z)
    // col3: {m12, m13, m14}  -> Translation (position)

    [[nodiscard]] inline Vector3 DegreesToRadians(const Vector3& rotation_in_degrees)
    {
        return Vector3Scale(rotation_in_degrees, DEG2RAD);
    }

    [[nodiscard]] inline Vector3 GetForwardVector(const Matrix& matrix)
    {
        // Extract the forward vector from the matrix's 3rd column (negated for -Z forward)
        const Vector3 forward = {-matrix.m8, -matrix.m9, -matrix.m10};
        return Vector3Normalize(forward);
    }

    [[nodiscard]] inline Vector3 GetRightVector(const Matrix& matrix)
    {
        // Extract the right vector from the matrix's 1st column
        const Vector3 right = {matrix.m0, matrix.m1, matrix.m2};
        return Vector3Normalize(right);
    }

    [[nodiscard]] inline Vector3 GetUpVector(const Matrix& matrix)
    {
        // Extract the up vector from the matrix's 2nd column
        const Vector3 up = {matrix.m4, matrix.m5, matrix.m6};
        return Vector3Normalize(up);
    }

    [[nodiscard]] inline Vector3 GetPositionFromMatrix(const Matrix& matrix)
    {
        // Extract the position vector from the matrix's 4th column
        const Vector3 position = {matrix.m12, matrix.m13, matrix.m14};
        return position;
    }

    [[nodiscard]] inline Vector3 GetTargetForCamera(const Matrix& matrix)
    {
        return GetPositionFromMatrix(matrix) + GetForwardVector(matrix);
    }
}
