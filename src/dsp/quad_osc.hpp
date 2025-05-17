#pragma once

namespace ogun {

class QuadOSC {
public:
    void Reset() { u_ = 1.0f; v_ = 0.0f; }
    void SetFreq(float omega);
    void Tick();
    float GetSine() const { return u_; }
    float GetCosine() const { return v_; }
private:
    float k1_{};
    float k2_{};
    float u_{};
    float v_{};
};

}
