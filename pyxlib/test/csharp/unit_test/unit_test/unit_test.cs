
public class unit_test
{
	static int Main()
	{
        PYXLibInstance.initialize("unit_test (C#)");

        // Run unit tests
        bool bSuccess = TestFrame.getInstance().testUnit();

        PYXLibInstance.destroy();

        return bSuccess ? 0 : 1;
    }
}
