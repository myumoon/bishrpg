// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "QuadTreeTest.h"

#include "AutomationTest.h"
#include "Algorithm/QuadTree.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FQuadTreeTest, "BiSHRPGTest.QuadTreeTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuadTreeTest::RunTest(const FString& Parameters)
{
	const FVector offset = { -0.001f, -0.001f, 0.0f }; // 線上用のオフセット
	//const FVector offset = { 0.0f, 0.0f, 0.0f}; // 線上用のオフセット
	const FVector begin  = { 0.0f, 0.0f, 0.0f };
	const FVector end    = { 8.0f, 8.0f, 0.0f };
	const int32   separateLevel = 3;
	QuadTree quadTree(begin + offset, end + offset, separateLevel);
	
	TestTrue("IsValid", quadTree.IsValid());
	TestEqual("LinearSpaceSize", QuadTree::GetLinearSpaceSize(4), 341);
	
	TestEqual("OutOfRange", quadTree.CalcMortonIndex(FVector(0.0f, -9999.0f, 0.0f)),-1);
	TestEqual("OutOfRange", quadTree.CalcMortonIndex(FVector(0.0f, 9999.0f, 0.0f)), -1);
	TestEqual("OutOfRange", quadTree.CalcMortonIndex(FVector(-9999.0f, 0.0, 0.0f)), -1);
	TestEqual("OutOfRange", quadTree.CalcMortonIndex(FVector(9999.0f, 0.0f, 0.0f)), -1);
	
	TestEqual("Morton-LeftUp",    quadTree.CalcMortonIndex(FVector(0.0f, 0.0f, 0.0f)), 0);
	TestEqual("Morton-RightUp",   quadTree.CalcMortonIndex(FVector(7.0f, 0.0f, 0.0f)), 21);
	TestEqual("Morton-LeftDown",  quadTree.CalcMortonIndex(FVector(0.0f, 7.0f, 0.0f)), 42);
	TestEqual("Morton-RightDown", quadTree.CalcMortonIndex(FVector(7.0f, 7.0f, 0.0f)), 63);

	TestEqual("Morton-Center", quadTree.CalcMortonIndex(FVector(3.0f, 3.0f, 0.0f)), 15);
	TestEqual("Morton-Center", quadTree.CalcMortonIndex(FVector(4.0f, 3.0f, 0.0f)), 26);
	TestEqual("Morton-Center", quadTree.CalcMortonIndex(FVector(3.0f, 4.0f, 0.0f)), 37);
	TestEqual("Morton-Center", quadTree.CalcMortonIndex(FVector(4.0f, 4.0f, 0.0f)), 48);

	constexpr int32 Level3Offset = 21;
	TestEqual("LinearMortonIndex-LeftUp",    quadTree.CalcLinearSpaceMortonIndex(FVector(0.0f, 0.0f, 0.0f)), Level3Offset + 0);
	TestEqual("LinearMortonIndex-RightUp",   quadTree.CalcLinearSpaceMortonIndex(FVector(7.0f, 0.0f, 0.0f)), Level3Offset + 21);
	TestEqual("LinearMortonIndex-LeftDown",  quadTree.CalcLinearSpaceMortonIndex(FVector(0.0f, 7.0f, 0.0f)), Level3Offset + 42);
	TestEqual("LinearMortonIndex-RightDown", quadTree.CalcLinearSpaceMortonIndex(FVector(7.0f, 7.0f, 0.0f)), Level3Offset + 63);

	TestEqual("LinearMortonIndex-Center-LeftUp",    quadTree.CalcLinearSpaceMortonIndex(FVector(3.0f, 3.0f, 0.0f)), Level3Offset + 15);
	TestEqual("LinearMortonIndex-Center-RightUp",   quadTree.CalcLinearSpaceMortonIndex(FVector(4.0f, 3.0f, 0.0f)), Level3Offset + 26);
	TestEqual("LinearMortonIndex-Center-LeftDown",  quadTree.CalcLinearSpaceMortonIndex(FVector(3.0f, 4.0f, 0.0f)), Level3Offset + 37);
	TestEqual("LinearMortonIndex-Center-RightDown", quadTree.CalcLinearSpaceMortonIndex(FVector(4.0f, 4.0f, 0.0f)), Level3Offset + 48);

	TestEqual("LinearMortonIndex-Level0", quadTree.CalcLinearSpaceIndex(FVector(0.0f, 0.0f, 0.0f), FVector(4.0f, 4.0f, 0.0f)), 0);
	TestEqual("LinearMortonIndex-Level0", quadTree.CalcLinearSpaceIndex(FVector(3.0f, 3.0f, 0.0f), FVector(4.0f, 4.0f, 0.0f)), 0);
	TestEqual("LinearMortonIndex-Level0", quadTree.CalcLinearSpaceIndex(FVector(0.0f, 0.0f, 0.0f), FVector(7.0f, 7.0f, 0.0f)), 0);

	TestEqual("LinearMortonIndex-Level1", quadTree.CalcLinearSpaceIndex(FVector(0.0f, 1.0f, 0.0f), FVector(2.0f, 3.0f, 0.0f)), 1);
	TestEqual("LinearMortonIndex-Level1", quadTree.CalcLinearSpaceIndex(FVector(5.0f, 1.0f, 0.0f), FVector(7.0f, 3.0f, 0.0f)), 2);
	TestEqual("LinearMortonIndex-Level1", quadTree.CalcLinearSpaceIndex(FVector(2.0f, 5.0f, 0.0f), FVector(3.0f, 7.0f, 0.0f)), 3);
	TestEqual("LinearMortonIndex-Level1", quadTree.CalcLinearSpaceIndex(FVector(4.0f, 4.0f, 0.0f), FVector(7.0f, 7.0f, 0.0f)), 4);

	TestEqual("LinearMortonIndex-Level2", quadTree.CalcLinearSpaceIndex(FVector(0.0f, 0.0f, 0.0f), FVector(1.0f, 1.0f, 0.0f)), 5);
	TestEqual("LinearMortonIndex-Level2", quadTree.CalcLinearSpaceIndex(FVector(2.0f, 2.0f, 0.0f), FVector(3.0f, 2.5f, 0.0f)), 8);
	TestEqual("LinearMortonIndex-Level2", quadTree.CalcLinearSpaceIndex(FVector(4.0f, 2.0f, 0.0f), FVector(5.0f, 3.0f, 0.0f)), 11);
	TestEqual("LinearMortonIndex-Level2", quadTree.CalcLinearSpaceIndex(FVector(4.0f, 4.0f, 0.0f), FVector(5.0f, 5.0f, 0.0f)), 17);

	TestEqual("LinearMortonIndex-Level3", quadTree.CalcLinearSpaceIndex(FVector(0.0f, 0.0f, 0.0f), FVector(0.5f, 0.5f, 0.0f)), Level3Offset + 0);
	TestEqual("LinearMortonIndex-Level3", quadTree.CalcLinearSpaceIndex(FVector(7.0f, 0.0f, 0.0f), FVector(7.5f, 0.5f, 0.0f)), Level3Offset + 21);
	TestEqual("LinearMortonIndex-Level3", quadTree.CalcLinearSpaceIndex(FVector(0.0f, 7.0f, 0.0f), FVector(0.5f, 7.5f, 0.0f)), Level3Offset + 42);
	TestEqual("LinearMortonIndex-Level3", quadTree.CalcLinearSpaceIndex(FVector(7.0f, 7.0f, 0.0f), FVector(7.5f, 7.5f, 0.0f)), Level3Offset + 63);
	TestEqual("LinearMortonIndex-Level3", quadTree.CalcLinearSpaceIndex(FVector(3.0f, 3.0f, 0.0f), FVector(3.5f, 3.5f, 0.0f)), Level3Offset + 15);
	TestEqual("LinearMortonIndex-Level3", quadTree.CalcLinearSpaceIndex(FVector(4.0f, 3.0f, 0.0f), FVector(4.5f, 3.5f, 0.0f)), Level3Offset + 26);
	TestEqual("LinearMortonIndex-Level3", quadTree.CalcLinearSpaceIndex(FVector(3.0f, 4.0f, 0.0f), FVector(3.5f, 4.5f, 0.0f)), Level3Offset + 37);
	TestEqual("LinearMortonIndex-Level3", quadTree.CalcLinearSpaceIndex(FVector(4.0f, 4.0f, 0.0f), FVector(4.5f, 4.5f, 0.0f)), Level3Offset + 48);

	// 範囲内と範囲外をまたぐ
	TestEqual("Range-InOut", quadTree.CalcLinearSpaceIndex(FVector(-1.0f, -1.0f, 0.0f), FVector(2.0f, 2.0f, 0.0f)), 1);
	TestEqual("Range-InOut", quadTree.CalcLinearSpaceIndex(FVector(5.0f, -1.0f, 0.0f), FVector(8.0f, 3.0f, 0.0f)), 2);
	TestEqual("Range-InOut", quadTree.CalcLinearSpaceIndex(FVector(-1.0f, 5.0f, 0.0f), FVector(2.0f, 7.0f, 0.0f)), 3);
	TestEqual("Range-InOut", quadTree.CalcLinearSpaceIndex(FVector(4.0f, 4.0f, 0.0f), FVector(8.0f, 8.0f, 0.0f)), 4);

	// 全体を覆っている
	TestEqual("Range-Whole", quadTree.CalcLinearSpaceIndex(FVector(-1.0f, -1.0f, 0.0f), FVector(8.0f, 8.0f, 0.0f)), 0);

	// 範囲外
	TestEqual("Range-Out", quadTree.CalcLinearSpaceIndex(FVector(-10.0f, -10.0f, 0.0f), FVector(-5.0f, -5.0f, 0.0f)), -1);

	return true;
}

