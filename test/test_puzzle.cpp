#include <puzzle.h>
#include <gtest/gtest.h>

unsigned char randomPuzzleValue(unsigned char puzzleSize, bool includeZero=true) {
	if (includeZero)
		return static_cast<char>(std::rand() % (puzzleSize + 1));
	return static_cast<char>((std::rand() % puzzleSize) + 1);
}

namespace {

class PuzzleTest : public ::testing::Test {
	protected:	
		Puzzle puzzle;
		unsigned char puzzleSizeSqrt, puzzleSize, puzzleSizeSquared, puzzleNeighborhoodSize;
	public:
		PuzzleTest() {
			puzzleSizeSqrt = 3;
			puzzleSize = 9;
			puzzleSizeSquared = 81;
			puzzleNeighborhoodSize = 20;
			unsigned char *values = new unsigned char[puzzleSize * puzzleSize] {
				1, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 2, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 3, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 4, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 5, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 6, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 7, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 8, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 9
			};
			Puzzle temp = Puzzle(puzzleSize, values);
			puzzle.swap(temp);

			delete [] values;
		}
		~PuzzleTest() {
		}
};


TEST_F(PuzzleTest, TestInitializedValues) {
	for (int r = 0, c = 0; r < puzzleSize; c = (c+1) % puzzleSize, r = r + (c == 0)) {
		char eVal = (r == c ? r+1 : 0);
		EXPECT_EQ(puzzle.getValue(r, c), eVal);
	}
}

TEST_F(PuzzleTest, TestConcreteValues) {
	for (int r = 0, c = 0; r < puzzleSize; c = (c+1) % puzzleSize, r = r + (c == 0)) {
		char eVal = (r == c ? 1 : 0);
		EXPECT_EQ(puzzle.isConcrete(r, c), eVal);
	}
}

TEST_F(PuzzleTest, TestConcreteValuesUnsettable) {
	for (int r = 0; r < puzzleSize; r++) {
		char rVal = randomPuzzleValue(puzzleSize);
		EXPECT_EQ(puzzle.setValue(r, r, rVal), false);
		EXPECT_EQ(puzzle.getValue(r, r), r+1);
	}
}

TEST_F(PuzzleTest, TestNonConcreteValuesSettable) {
	for (int r = 0, c = 0; r < puzzleSize; c = (c+1) % puzzleSize, r = r + (c == 0)) {
		if (r == c) continue;
		char rVal = randomPuzzleValue(puzzleSize);
		EXPECT_EQ(puzzle.setValue(r, c, rVal), true);
		EXPECT_EQ(puzzle.getValue(r, c), rVal);
	}
}

TEST_F(PuzzleTest, TestOOBValuesUnsettable) {
	char settableCoords[][2] = {
		{0, 1},
		{5, 0},
		{8, 7}
	};
	char passVals[] = {0, 4, 9};
	char failVals[] = {10, 32, 120};

	for (char* c : settableCoords) {
		for (char val : failVals) {
			EXPECT_EQ(puzzle.setValue(c[0], c[1], val), false);
			EXPECT_EQ(puzzle.getValue(c[0], c[1]), 0);
		}
		for (char val : passVals) {
			EXPECT_EQ(puzzle.setValue(c[0], c[1], val), true);
			EXPECT_EQ(puzzle.getValue(c[0], c[1]), val);
		}
	}
}

TEST_F(PuzzleTest, TestSetEmpty) {
	for (int r = 0, c = 0; r < puzzleSize; c = (c+1) % puzzleSize, r = r + (c == 0)) {
		if (r == c) continue;
		// Fill with nonzero values
		char rVal = randomPuzzleValue(puzzleSize, false);
		EXPECT_EQ(puzzle.setValue(r, c, rVal), 1);
		ASSERT_NE(puzzle.getValue(r, c), 0);
		EXPECT_EQ(puzzle.setValue(r, c, 0), 1);
		EXPECT_EQ(puzzle.getValue(r, c), 0);
	}
}

} // namespace