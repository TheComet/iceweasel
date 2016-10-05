class RotateLight : ScriptObject
{
    void Update(float timeStep)
    {
        if(input.keyPress[KEY_R])
            doRotate_ = !doRotate_;
        if(doRotate_ == false)
            return;
        angle_ = angle_ + speed_ * timeStep;
        Vector2 position;
        position.x = radius * Cos(angle_);
        position.y = radius * Sin(angle_);
        position = position + center_;
        Vector3 position3D = Vector3(position.x, height, position.y);
        node.set_position(position3D);
    }

    float angle_ = 0.0;
    float speed_ = 5.0f;
    Vector2 center_ = Vector2(0, 0);
    float radius = 8;
    float height = 5;
    bool doRotate_ = false;
}
