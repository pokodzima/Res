#pragma once
#include "raylib.h"
#include "raymath.h"

namespace res
{
    inline Vector3 DegreesToRadians(const Vector3 RotationInDegrees)
    {
        const Vector3 RotationInRadians{
            RotationInDegrees.x * DEG2RAD,
            RotationInDegrees.y * DEG2RAD,
            RotationInDegrees.z * DEG2RAD
        };
        return RotationInRadians;
    }

    inline Vector3 GetForwardVector(const Matrix& matrix)
    {
        // Extract the forward vector from the matrix's 3rd column (negated for -Z forward)
        const Vector3 forward = {-matrix.m8, -matrix.m9, -matrix.m10};
        return Vector3Normalize(forward);
    }

    inline Vector3 GetRightVector(const Matrix& matrix)
    {
        // Extract the right vector from the matrix's 1st column
        const Vector3 right = {matrix.m0, matrix.m1, matrix.m2};
        return Vector3Normalize(right);
    }

    inline Vector3 GetUpVector(const Matrix& matrix)
    {
        // Extract the up vector from the matrix's 2nd column
        const Vector3 up = {matrix.m4, matrix.m5, matrix.m6};
        return Vector3Normalize(up);
    }

    inline Vector3 GetPositionFromMatrix(const Matrix& matrix)
    {
        // Extract the position vector from the matrix's 4th column
        const Vector3 position = {matrix.m12, matrix.m13, matrix.m14};
        return position;
    }

    inline Vector3 GetTargetForCamera(const Matrix& matrix)
    {
        return GetPositionFromMatrix(matrix) + GetForwardVector(matrix);
    }
}
