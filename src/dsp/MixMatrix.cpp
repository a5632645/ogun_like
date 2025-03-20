#include "MixMatrix.hpp"
#include <numbers>

namespace mana {

std::array<float, MixMatrix::kSize> MixMatrix::Process(std::array<float, MixMatrix::kSize> in) const {
    std::array<float, MixMatrix::kSize> out{};
    for (int i = 0; i < MixMatrix::kSize; ++i) {
        for (int j = 0; j < MixMatrix::kSize; ++j) {
            out[i] += matrix_[i][j] * in[j];
        }
    }
    return out;
}

void MixMatrix::MakeHammond() {
    matrix_[0][0] = 0.5f;
    matrix_[0][1] = 0.5f;
    matrix_[0][2] = 0.5f;
    matrix_[0][3] = 0.5f;

    matrix_[1][0] = 0.5f;
    matrix_[1][1] = 0.5f;
    matrix_[1][2] = -0.5f;
    matrix_[1][3] = -0.5f;

    matrix_[2][0] = 0.5f;
    matrix_[2][1] = -0.5f;
    matrix_[2][2] = 0.5f;
    matrix_[2][3] = -0.5f;

    matrix_[3][0] = 0.5f;
    matrix_[3][1] = -0.5f;
    matrix_[3][2] = -0.5f;
    matrix_[3][3] = 0.5f;
}

void MixMatrix::MakeHammondSplit() {
    MakeZero();
    constexpr auto invSqrt2 = 1.0f / std::numbers::sqrt2_v<float>;
    matrix_[0][0] = invSqrt2;
    matrix_[1][0] = invSqrt2;
    matrix_[0][1] = invSqrt2;
    matrix_[1][1] = -invSqrt2;

    matrix_[2][2] = invSqrt2;
    matrix_[2][3] = invSqrt2;
    matrix_[3][2] = invSqrt2;
    matrix_[3][3] = -invSqrt2;
}

void MixMatrix::MakeRandom() {

}

void MixMatrix::MakeZero() {
    for (int i = 0; i < MixMatrix::kSize; ++i) {
        for (int j = 0; j < MixMatrix::kSize; ++j) {
            matrix_[i][j] = 0;
        }
    }
}

void MixMatrix::MakeUnit() {
    MakeZero();
    for (int i = 0; i < MixMatrix::kSize; ++i) {
        matrix_[i][i] = 1;
    }
}
}