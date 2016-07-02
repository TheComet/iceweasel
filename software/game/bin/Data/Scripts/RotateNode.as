class RotateNode : ScriptObject
{
    void Update(float timeStep)
    {
        float speed = 60.0f;
        if(input.keyDown[KEY_R])
            node.Rotate(Quaternion(timeStep * speed, Vector3(0.0f, 1.0f, 0.0f)));
    }
}
