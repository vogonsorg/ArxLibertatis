/*
 * Copyright 2011-2020 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "math/LegacyMathTest.h"

#include <sstream>

#include "game/Camera.h"
#include "graphics/Math.h"
#include "math/GtxFunctions.h"

#include "math/AssertionTraits.h"
#include "math/LegacyMath.h"

CppUnit::AutoRegisterSuite<LegacyMathTest> g_registerLegacyMathTest;

struct TestRotation {
	
	glm::quat quat;
	glm::mat3 mat;
	
	TestRotation(glm::quat quat_, glm::mat3 mat_) noexcept
		: quat(quat_)
		, mat(mat_)
	{ }
	
};

std::vector<TestRotation> rotations;
std::vector<Anglef> angles;

static void addTestData(glm::quat quat, glm::mat3 mat) {
	rotations.emplace_back(quat, glm::transpose(mat));
}

void LegacyMathTest::setUp() {
	
	using glm::quat;
	using glm::mat3;
	
	// Data from:
	// https://euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/steps/index.htm
	// https://euclideanspace.com/maths/algebra/matrix/transforms/examples/index.htm
	
	// Identity (no rotation)
	addTestData(quat(    1.f,    0.0f,    0.0f,    0.0f), mat3( 1, 0, 0,  0, 1, 0,  0, 0, 1));
	// 90 degrees about y axis
	addTestData(quat(0.7071f,    0.0f, 0.7071f,    0.0f), mat3( 0, 0, 1,  0, 1, 0, -1, 0, 0));
	// 180 degrees about y axis
	addTestData(quat(   0.0f,    0.0f,     1.f,    0.0f), mat3(-1, 0, 0,  0, 1, 0,  0, 0,-1));
	// 270 degrees about y axis
	addTestData(quat(0.7071f,    0.0f,-0.7071f,    0.0f), mat3( 0, 0,-1,  0, 1, 0,  1, 0, 0));
	
	addTestData(quat(0.7071f,    0.0f,    0.0f, 0.7071f), mat3( 0,-1, 0,  1, 0, 0,  0, 0, 1));
	addTestData(quat(   0.5f,    0.5f,    0.5f,    0.5f), mat3( 0, 0, 1,  1, 0, 0,  0, 1, 0));
	addTestData(quat(   0.0f, 0.7071f, 0.7071f,    0.0f), mat3( 0, 1, 0,  1, 0, 0,  0, 0,-1));
	addTestData(quat(   0.5f,   -0.5f,   -0.5f,    0.5f), mat3( 0, 0,-1,  1, 0, 0,  0,-1, 0));
	
	addTestData(quat(0.7071f,    0.0f,    0.0f,-0.7071f), mat3( 0, 1, 0, -1, 0, 0,  0, 0, 1));
	addTestData(quat(   0.5f,   -0.5f,    0.5f,   -0.5f), mat3( 0, 0, 1, -1, 0, 0,  0,-1, 0));
	addTestData(quat(   0.0f,-0.7071f, 0.7071f,    0.0f), mat3( 0,-1, 0, -1, 0, 0,  0, 0,-1));
	addTestData(quat(   0.5f,    0.5f,   -0.5f,   -0.5f), mat3( 0, 0,-1, -1, 0, 0,  0, 1, 0));
	
	addTestData(quat(0.7071f, 0.7071f,    0.0f,    0.0f), mat3( 1, 0, 0,  0, 0,-1,  0, 1, 0));
	addTestData(quat(   0.5f,    0.5f,    0.5f,   -0.5f), mat3( 0, 1, 0,  0, 0,-1, -1, 0, 0));
	addTestData(quat(   0.0f,    0.0f, 0.7071f,-0.7071f), mat3(-1, 0, 0,  0, 0,-1,  0,-1, 0));
	addTestData(quat(   0.5f,    0.5f,   -0.5f,    0.5f), mat3( 0,-1, 0,  0, 0,-1,  1, 0, 0));
	
	addTestData(quat(   0.0f,    1.0f,    0.0f,    0.0f), mat3( 1, 0, 0,  0,-1, 0,  0, 0,-1));
	addTestData(quat(   0.0f, 0.7071f,    0.0f,-0.7071f), mat3( 0, 0,-1,  0,-1, 0, -1, 0, 0));
	addTestData(quat(   0.0f,    0.0f,    0.0f,    1.0f), mat3(-1, 0, 0,  0,-1, 0,  0, 0, 1));
	addTestData(quat(   0.0f, 0.7071f,    0.0f, 0.7071f), mat3( 0, 0, 1,  0,-1, 0,  1, 0, 0));
	
	addTestData(quat(0.7071f,-0.7071f,    0.0f,    0.0f), mat3( 1, 0, 0,  0, 0, 1,  0,-1, 0));
	addTestData(quat(   0.5f,   -0.5f,    0.5f,    0.5f), mat3( 0,-1, 0,  0, 0, 1, -1, 0, 0));
	addTestData(quat(   0.0f,    0.0f, 0.7071f, 0.7071f), mat3(-1, 0, 0,  0, 0, 1,  0, 1, 0));
	addTestData(quat(   0.5f,   -0.5f,   -0.5f,   -0.5f), mat3( 0, 1, 0,  0, 0, 1,  1, 0, 0));
	
	for(int i = 180; i > -180; i -= 15) {
		for(int j = 180; j > -180; j -= 15) {
			for(int k = 180; k > -180; k -= 15) {
				angles.emplace_back(float(i), float(j), float(k));
			}
		}
	}
	
}

void LegacyMathTest::tearDown() {
	rotations.clear();
}

void LegacyMathTest::rotationTestDataTest() {
	for(const TestRotation & rotation : rotations) {
		CPPUNIT_ASSERT_EQUAL(rotation.quat, glm::quat_cast(rotation.mat));
		CPPUNIT_ASSERT_EQUAL(glm::mat4_cast(rotation.quat), glm::mat4(rotation.mat));
	}
}

void LegacyMathTest::quaternionTests() {
	
	for(const TestRotation & rotation : rotations) {
		
		glm::quat A = rotation.quat;
		glm::quat B = rotation.quat;
	
		CPPUNIT_ASSERT_EQUAL(A, B);
		
		glm::quat inverseA = A;
		Quat_Reverse(&inverseA);
		glm::quat inverseB = glm::inverse(B);
		
		CPPUNIT_ASSERT_EQUAL(inverseA, inverseB);
	
		Vec3f vecA = TransformVertexQuat(A, Vec3f(1.f, 0.5f, 0.1f));
		Vec3f vecB = B * Vec3f(1.f, 0.5f, 0.1f);
		
		CPPUNIT_ASSERT_EQUAL(vecA, vecB);
		
		glm::mat4x4 matrixA(1.f);
		MatrixFromQuat(matrixA, A);
		
		glm::mat4x4 matrixB = glm::mat4_cast(B);
		
		CPPUNIT_ASSERT_EQUAL(matrixA, matrixB);
		
	}
	
}

void LegacyMathTest::quatMuliplyTest() {
	glm::quat A = glm::quat(0.f,  1.f, 0.f, 0.f);
	glm::quat B = glm::quat(0.f,  0.f, 1.f, 0.f);
	
	CPPUNIT_ASSERT_EQUAL(A * B, Quat_Multiply(A, B));
}

// TODO this does not seem to cover all edge cases
//void LegacyMathTest::quatSlerpTest() {
//	
//	typedef std::vector<TestRotation>::iterator itr;
//	for(itr iA = rotations.begin(); iA != rotations.end(); ++iA) {
//		for(itr iB = rotations.begin(); iB != rotations.end(); ++iB) {
//			for(int a = 0; a <= 10; a++) {
//				glm::quat expected = glm::slerp(iA->quat, iB->quat, a * 0.1f);
//				glm::quat result = Quat_Slerp(iA->quat, iB->quat, a * 0.1f);
//				
//				CPPUNIT_ASSERT_EQUAL(expected, result);
//			}
//		}
//	}
//}

void LegacyMathTest::quatTransformVectorTest() {
	
	for(const TestRotation & rotation : rotations) {
		
		const Vec3f testVert(1.f, 0.5f, 0.1f);
		
		Vec3f vecA = TransformVertexQuat(rotation.quat, testVert);
		Vec3f vecB = rotation.quat * testVert;
		
		CPPUNIT_ASSERT_EQUAL(vecA, vecB);
		
		Vec3f vecC;
		TransformInverseVertexQuat(rotation.quat, testVert, vecC);
		
		Vec3f vecD = glm::inverse(rotation.quat) * testVert;
		
		CPPUNIT_ASSERT_EQUAL(vecC, vecD);
		
	}
	
}

void LegacyMathTest::quatMatrixConversionTest() {
	
	for(const TestRotation & rotation : rotations) {
		
		CPPUNIT_ASSERT_EQUAL(glm::mat4_cast(rotation.quat), glm::mat4(rotation.mat));
		
		glm::quat q;
		QuatFromMatrix(q, glm::mat4(rotation.mat));
		CPPUNIT_ASSERT_EQUAL(glm::quat_cast(rotation.mat), q);
		
	}
	
}

void LegacyMathTest::vecMatrixConversionTest() {
	
	for(const TestRotation & rotation : rotations) {
		
		Vec3f front(0, 0, 1);
		Vec3f up(0, 1, 0);
		
		front = rotation.quat * front;
		up = rotation.quat * up;
		
		glm::mat4 mat(1.f);
		MatrixSetByVectors(mat, front, up);
		
		CPPUNIT_ASSERT_EQUAL(glm::mat4(rotation.mat), mat);
		
	}
	
}

void LegacyMathTest::angleConversionTest() {
	
	for(Anglef angle : angles) {
		
		glm::quat q = toNonNpcRotation(angle);
		
		glm::quat q2 = glm::quat_cast(toRotationMatrix(angle));
		
		glm::quat q3 = toQuaternion(angle);
		
		glm::quat q4 = toQuaternion(toAngle(q3));
		
		CPPUNIT_ASSERT_EQUAL(q, q2);
		
		CPPUNIT_ASSERT_EQUAL(q, q3);
		
		CPPUNIT_ASSERT_EQUAL(q3, q4);
		
	}
	
}

// TODO copy-paste
static Vec2s inventorySizeFromTextureSize(Vec2i size) {
	return Vec2s(glm::clamp((size + Vec2i(31, 31)) / Vec2i(32, 32), Vec2i(1, 1), Vec2i(3, 3)));
}

void LegacyMathTest::inventorySizeTest() {
	
	for(short i = 0; i < 100; ++i)
	for(short j = 0; j < 100; ++j) {
		Vec2i size(i, j);
		Vec2s expected = inventorySizeFromTextureSize(size);
		Vec2s oldResult = inventorySizeFromTextureSize_2(i, j);
		
		CPPUNIT_ASSERT_EQUAL_MESSAGE(CPPUNIT_NS::assertion_traits<Vec2i>::toString(Vec2i(i, j)), expected, oldResult);
	}
}

void LegacyMathTest::angleToVectorXZ_Test() {
	
	for(int i = -100000; i < 100000; i++) {
		float angle = float(i) * 0.01f;
		Vec3f expected = angleToVectorXZ(angle);
		Vec3f result = angleToVectorXZ_180offset(angle + 180);
		
		Vec3f result2 = VRotateY(Vec3f(0.f, 0.f, 1.f), 360.f - angle);
		
		CPPUNIT_ASSERT_EQUAL(expected, result);
		CPPUNIT_ASSERT_EQUAL(expected, result2);
	}
}

void LegacyMathTest::vectorRotateTest() {
	
	for(size_t i = 0; i < 720; i += 10) {
		float angle = float(i);
		
		Vec3f foo = Vec3f(0.f, 0.f, 1.f);
		
		Vec3f result;
		VectorRotateY(foo, result, glm::radians(angle));
		
		Vec3f result2 = VRotateY(foo, angle);
		
		CPPUNIT_ASSERT_EQUAL(result, result2);
	}
	
	for(size_t i = 0; i < 720; i += 10) {
		float angle = float(i);
		
		Vec3f foo = Vec3f(1.f, 0.f, 0.f);
		
		Vec3f result;
		VectorRotateZ(foo, result, glm::radians(angle));
		
		Vec3f result2 = VRotateZ(foo, angle);
		
		CPPUNIT_ASSERT_EQUAL(result, result2);
	}
}

void LegacyMathTest::focalToFovTest() {
	
	for(size_t i = 1000; i < 8000; i++) {
		float focal = float(i) * 0.1f;
		
		float expected = glm::radians(focalToFovLegacy(focal));
		float result = Camera::focalToFov(focal);
		
		std::ostringstream ss;
		ss << "In: " << focal;
		ss << " Expected: " << expected;
		ss << ", Result: " << result;
		CPPUNIT_ASSERT_MESSAGE(ss.str(), glm::epsilonEqual(expected, result, 2.f));
	}
}

void LegacyMathTest::pointInerpolationTest() {
	
	for(int i = 0; i < 500; i++) {
		Vec3f v0 = glm::linearRand(Vec3f(-10), Vec3f(10));
		Vec3f v1 = glm::linearRand(Vec3f(-10), Vec3f(10));
		Vec3f v2 = glm::linearRand(Vec3f(-10), Vec3f(10));
		Vec3f v3 = glm::linearRand(Vec3f(-10), Vec3f(10));
		
		for(int u = 0; u < 1000; u++) {
			float f = u / 1000.f;
			
			Vec3f res1 = interpolatePos(f, v0, v1, v2, v3);
			Vec3f res2 = arx::catmullRom(v0, v1, v2, v3, f);
			
			CPPUNIT_ASSERT_EQUAL(res1, res2);
		}
	}
}
