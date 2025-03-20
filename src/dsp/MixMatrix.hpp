#pragma once
#include <array>

namespace mana {

class MixMatrix {
public:
    static constexpr int kSize = 4;

    MixMatrix() { MakeUnit(); }

    void MakeHammond();
    void MakeHammondSplit();
    void MakeRandom();
    void MakeUnit();
    void MakeZero();
    std::array<float, kSize> Process(std::array<float, kSize> in) const;
private:
    std::array<std::array<float, kSize>, kSize> matrix_{};
};

}