#pragma once

class ICurve
{
public:
    ICurve(float initialValue);
    virtual float Advance(float step) = 0;
    float GetValue() const;

protected:
    float value_;
};

class ExponentialCurve : public ICurve
{
public:
    ExponentialCurve();
    ExponentialCurve(float initialValue, float targetValue);

    void SetTarget(float targetValue);
    virtual float Advance(float step);

private:
    float targetValue_;
};
