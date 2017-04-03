template <class T>
class ICurve
{
public:
    ICurve(T initialValue);
    virtual T Advance(float step) = 0;

    T value_;
};


template <class T>
class ExponentialCurve : public ICurve<T>
{
public:
    ExponentialCurve();
    ExponentialCurve(T initialValue, T targetValue);

    void SetTarget(T targetValue);
    virtual T Advance(float step);

private:
    T targetValue_;
};
