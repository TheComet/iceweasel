class Gun : ScriptObject
{
    void Start()
    {
        node.set_position(Vector3(0.5, -0.5, 1.2));
    }

    void Update(float timeStep)
    {
        counter_ += timeStep;
        if(input.mouseButtonDown[MOUSEB_LEFT])
        {
            if(counter_ > 0.25) // 250ms between shots
            {
                //recoil_ = -50;
                counter_ = 0;

                // play recoil animation
                AnimationController@ controller = node.GetComponent("AnimationController");
                controller.Play("Models/Gun_PewPew.ani", 0, false, 0.0f);
                controller.SetSpeed("Models/Gun_PewPew.ani", 1);
                controller.SetTime("Models/Gun_PewPew.ani", 0);
            }
        }

        // slowly approach 0 again
        recoil_ += (0 - recoil_) * timeStep * 10;

        Quaternion recoilRotation = Quaternion(recoil_, Vector3(1, 0, 0));
        node.set_rotation(correctRotation_ * recoilRotation);
    }

    // Correct rotation of the gun so it is aligned properly
    private Quaternion correctRotation_ = Quaternion(0, Vector3(0, 1, 0));
    private float recoil_ = 0;
    private float targetRecoil_ = 0;
    private float counter_ = 0;
}

