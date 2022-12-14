#include <gtest/gtest.h>

#include "graph.h"

namespace 
{
    
TEST(TestGraph, NeighborhoodsTest) {
    const unsigned char rank = 9;
    unsigned ***neighborhoods = graphNeighborhoods(rank);
    unsigned truehoods[3][rank][rank] = {
        {
            { 0,  1,  2,  3,  4,  5,  6,  7,  8},
            { 9, 10, 11, 12, 13, 14, 15, 16, 17},
            {18, 19, 20, 21, 22, 23, 24, 25, 26},
            {27, 28, 29, 30, 31, 32, 33, 34, 35},
            {36, 37, 38, 39, 40, 41, 42, 43, 44},
            {45, 46, 47, 48, 49, 50, 51, 52, 53},
            {54, 55, 56, 57, 58, 59, 60, 61, 62},
            {63, 64, 65, 66, 67, 68, 69, 70, 71},
            {72, 73, 74, 75, 76, 77, 78, 79, 80}
        },
        {
            { 0,  9, 18, 27, 36, 45, 54, 63, 72},
            { 1, 10, 19, 28, 37, 46, 55, 64, 73},
            { 2, 11, 20, 29, 38, 47, 56, 65, 74},
            { 3, 12, 21, 30, 39, 48, 57, 66, 75},
            { 4, 13, 22, 31, 40, 49, 58, 67, 76},
            { 5, 14, 23, 32, 41, 50, 59, 68, 77},
            { 6, 15, 24, 33, 42, 51, 60, 69, 78},
            { 7, 16, 25, 34, 43, 52, 61, 70, 79},
            { 8, 17, 26, 35, 44, 53, 62, 71, 80}
        },
        {
            { 0,  1,  2,  9, 10, 11, 18, 19, 20},
            { 3,  4,  5, 12, 13, 14, 21, 22, 23},
            { 6,  7,  8, 15, 16, 17, 24, 25, 26},
            {27, 28, 29, 36, 37, 38, 45, 46, 47},
            {30, 31, 32, 39, 40, 41, 48, 49, 50},
            {33, 34, 35, 42, 43, 44, 51, 52, 53},
            {54, 55, 56, 63, 64, 65, 72, 73, 74},
            {57, 58, 59, 66, 67, 68, 75, 76, 77},
            {60, 61, 62, 69, 70, 71, 78, 79, 80}
        }
    };

    // Test content correctness
    for (char n = 0; n < 3; n++)
        for (char i = 0; i < rank; i++)
            for (char m = 0; m < rank; m++)
                EXPECT_EQ(neighborhoods[n][i][m], truehoods[n][i][m]);

    // Test cache use
    EXPECT_EQ(neighborhoods, graphNeighborhoods(rank));
}
    
TEST(TestGraph, NeigborhoodsByCellTest) {
    // Testing for rank 4
    unsigned ***neighborhoods = graphNeighborhoodByCell(4);
    unsigned truehoods[16][3][3] = {
        { { 1,  2,  3}, { 4,  8, 12}, { 1,  4,  5}, },
        { { 0,  2,  3}, { 5,  9, 13}, { 0,  4,  5}, },
        { { 0,  1,  3}, { 6, 10, 14}, { 3,  6,  7}, },
        { { 0,  1,  2}, { 7, 11, 15}, { 2,  6,  7}, },
        { { 5,  6,  7}, { 0,  8, 12}, { 0,  1,  5}, },
        { { 4,  6,  7}, { 1,  9, 13}, { 0,  1,  4}, },
        { { 4,  5,  7}, { 2, 10, 14}, { 2,  3,  7}, },
        { { 4,  5,  6}, { 3, 11, 15}, { 2,  3,  6}, },
        { { 9, 10, 11}, { 0,  4, 12}, { 9, 12, 13}, },
        { { 8, 10, 11}, { 1,  5, 13}, { 8, 12, 13}, },
        { { 8,  9, 11}, { 2,  6, 14}, {11, 14, 15}, },
        { { 8,  9, 10}, { 3,  7, 15}, {10, 14, 15}, },
        { {13, 14, 15}, { 0,  4,  8}, { 8,  9, 13}, },
        { {12, 14, 15}, { 1,  5,  9}, { 8,  9, 12}, },
        { {12, 13, 15}, { 2,  6, 10}, {10, 11, 15}, },
        { {12, 13, 14}, { 3,  7, 11}, {10, 11, 14}, }
    };

    // Test content correctness
    for (unsigned c = 0; c < 16; c++)
        for (char n = 0; n < 3; n++)
            for (char m = 0; m < 3; m++)
                EXPECT_EQ(neighborhoods[c][n][m], truehoods[c][n][m]);
    
    // Test cache use
    EXPECT_EQ(neighborhoods, graphNeighborhoodByCell(4));
}
    
TEST(TestGraph, NeighborsByCellTest) {
    const unsigned char rank = 4;
    unsigned **neighbors = graphNeighborsByCell(rank);
    unsigned truehoods[16][7] = {
        { 1,  2,  3,  4,  8, 12,  5},
        { 0,  2,  3,  5,  9, 13,  4},
        { 0,  1,  3,  6, 10, 14,  7},
        { 0,  1,  2,  7, 11, 15,  6},
        { 5,  6,  7,  0,  8, 12,  1},
        { 4,  6,  7,  1,  9, 13,  0},
        { 4,  5,  7,  2, 10, 14,  3},
        { 4,  5,  6,  3, 11, 15,  2},
        { 9, 10, 11,  0,  4, 12, 13},
        { 8, 10, 11,  1,  5, 13, 12},
        { 8,  9, 11,  2,  6, 14, 15},
        { 8,  9, 10,  3,  7, 15, 14},
        {13, 14, 15,  0,  4,  8,  9},
        {12, 14, 15,  1,  5,  9,  8},
        {12, 13, 15,  2,  6, 10, 11},
        {12, 13, 14,  3,  7, 11, 10}
    };

    // Test content correctness
    for (unsigned c = 0; c < 16; c++)
        for (char m = 0; m < 7; m++)
            EXPECT_EQ(neighbors[c][m], truehoods[c][m]);

    // Test cache use
    EXPECT_EQ(neighbors, graphNeighborsByCell(4));
}

} // namespace 
