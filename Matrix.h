#pragma once
#include"math/Matrix4x4.h"
#include "math/Vector3.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>



Matrix4x4 MakeIdentity4x4();
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 ret;
	ret.m[0][0] = 1.0f;
	ret.m[1][1] = 1.0f;
	ret.m[2][2] = 1.0f;
	ret.m[3][3] = 1.0f;
	return ret;
}

Matrix4x4 Multply(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 Multply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

//平行移動
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 ret;
	ret.m[0][0] = 1.0f;
	ret.m[1][1] = 1.0f;
	ret.m[2][2] = 1.0f;
	ret.m[3][3] = 1.0f;

	ret.m[3][0] = translate.x;
	ret.m[3][1] = translate.y;
	ret.m[3][2] = translate.z;
	return ret;
}

//拡大縮小
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 ret;
	ret.m[0][0] = scale.x;
	ret.m[1][1] = scale.y;
	ret.m[2][2] = scale.z;
	ret.m[3][3] = 1.0f;
	return ret;
}

// 1.X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian);
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 ret;
	ret.m[0][0] = 1.0f;
	ret.m[1][1] = std::cos(radian);
	ret.m[1][2] = std::sin(radian);
	ret.m[2][1] = std::sin(-radian);
	ret.m[2][2] = std::cos(radian);
	ret.m[3][3] = 1.0f;
	    
	return ret;
}
// 2.Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian);
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 ret;
	ret.m[0][0] = std::cos(radian);
	ret.m[0][2] = std::sin(-radian);
	ret.m[1][1] = 1.0f;
	ret.m[2][0] = std::sin(radian);
	ret.m[2][2] = std::cos(radian);
	ret.m[3][3] = 1.0f;
		//std::cos(radian), 0.0f, std::sin(-radian), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, std::sin(radian), 0.0f, std::cos(radian), 0.0f, 0.0f,0.0f, 0.0f, 1.0f;
	return ret;
}


// 3.Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian);
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 ret;
	ret.m[0][0] = std::cos(radian);
	ret.m[0][1] = std::sin(radian);
	ret.m[1][0] = std::sin(-radian);
	ret.m[1][1] = std::cos(radian);
	ret.m[2][2] = 1.0f;
	ret.m[3][3] = 1.0f;
	    /*std::cos(radian), std::sin(radian), 0.0f, 0.0f,
		std::sin(-radian), std::cos(radian), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f,*/
	
	return ret;
}

// 3次元アフィン変換行列
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateXYZMatrix = Multply(rotateXMatrix, Multply(rotateYMatrix, rotateZMatrix));
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
	Matrix4x4 ret = Multply(scaleMatrix, Multply(rotateXYZMatrix, translateMatrix));
	return ret;
}