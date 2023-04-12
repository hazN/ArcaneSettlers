class IDGenerator
{
public:
    static int GenerateID()
    {
        return mCurrentID++;
    }

private:
    static int mCurrentID;
};

