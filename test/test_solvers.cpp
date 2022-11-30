#include <tuple>
#include <gtest/gtest.h>

#include <solvers.h>
#include <solvers/graph_solvers.cpp> // included directly to test hidden structures

#define SIMPLEX(ndims, ...) buildSimplex(ndims, new double[ndims] __VA_ARGS__)
#define FLOAT_TOLERANCE 1e-8

simplex_data_t buildSimplex(unsigned char ndims, double vals[]) {
    simplex_data_t temp (ndims, vals);
    delete [] vals;
    return temp;
}

class SimplexDataTest : public ::testing::TestWithParam<simplex_data_t> {
    protected:
        // void SetUp() {
        // }
        // void TearDown() {
        // }
};

TEST_P(SimplexDataTest, TestConstrainAffine) {
    simplex_data_t data(GetParam());

    data.constrainAffine();

    double sum = 0;
    for (double *val = data.position; val < data.positionEnd; val++) sum += *val;
    
    EXPECT_NEAR(sum, 0, FLOAT_TOLERANCE);
}

// Test param options
// double ivals[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1}; double eval = 1. / 9; double evals[9] = {eVal, eVal, eVal, eVal, eVal, eVal, eVal, eVal, eVal};
// double ivals[9] = {1, 1, 0, 0, 0, 0, 0, 0, 0}; double evals[9] = {0.5, 0.5, 0, 0, 0, 0, 0, 0, 0};
// double ivals[9] = {1, 1, 0, 0, 0, -3, 0, 0, 0}; double evals[9] = {0.5, 0.5, 0, 0, 0, 0, 0, 0, 0};
TEST_P(SimplexDataTest, TestConstrainSimplex) {
    simplex_data_t update(GetParam());
    simplex_data_t data(9);
    double sum;

    update.constrainAffine();

    sum = 0;
    for (double *val = update.position; val < update.positionEnd; val++) sum += *val;
    ASSERT_NEAR(sum, 0, FLOAT_TOLERANCE);

    data += update;
    
    sum = 0;
    for (double *val = data.position; val < data.positionEnd; val++) sum += *val;
    ASSERT_NEAR(sum, 1, FLOAT_TOLERANCE);

    data.constrainSimplex();

    sum = 0;
    for (double *val = data.position; val < data.positionEnd; val++) sum += *val;
    EXPECT_NEAR(sum, 1, FLOAT_TOLERANCE);
}

INSTANTIATE_TEST_SUITE_P(
    SimplexDataTest,
    SimplexDataTest,
    ::testing::Values(
        SIMPLEX(9, {1, 1, 1, 1, 1, 1, 1, 1, 1}),
        SIMPLEX(9, {1, 1, 0, 0, 0, 0, 0, 0, 0}),
        SIMPLEX(9, {1, 1, 0, 0, -3, 0, 0, 0, 0})
    )
);