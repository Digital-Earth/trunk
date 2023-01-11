using System;
using System.Collections.Generic;
using System.Windows.Forms;

public class CSharpObserver : Observer
{
    protected override void updateObserverImpl(NotifierEvent_SPtr spEvent)
    {
        try
        {
            // Manually dereference smart pointer proxy and manually downcast underlying object.
            PYXCameraEvent camEvent = PYXCameraEvent.dynamic_cast(spEvent.__deref__());
            PYXCoord3DDouble coord = camEvent.getCamera().getState().getPosition();
            Trace.info("CSharpObserver.updateObserverImpl called with camera event with position " +
                coord.x() + "," + coord.y() + "," + coord.z());
        }
        catch (InvalidCastException e)
        {
            Trace.info("CSharpObserver.updateObserverImpl called with non-camera event and exception " + e.Message);
        }
    }
};

public class MyTraceCallback : TraceCallback
{
    public override void message(string strMessage)
    {
        Console.WriteLine(strMessage);
    }
};

namespace SampleApp
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            MyFunc();
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new SampleForm());
        }

        static void MyFunc()
        {
            // Init pyxlib
            PYXLibInstance.initialize("SampleApp");
            // Set trace callback
            MyTraceCallback myTraceCallback = new MyTraceCallback();
            Trace.setTraceCallback(myTraceCallback);

            NavigationModelInstance.initialize();

            // Trace
            Trace.time("Hello PYXIS Library!");

            // Run unit tests
            //TestFrame.getInstance().testUnit();

            // Index
            PYXIcosIndex i = new PYXIcosIndex("A-0202");
            Trace.info("i=" + i.toString());

            // Math
            PYXIcosIndex i2 = PYXIcosMath.move(i, PYXMath.eHexDirection.knDirectionTwo);
            Trace.info("i2=" + i2.toString());

            // Coordinates and projection
            CoordLatLon ll = new CoordLatLon();
            ll.setInDegrees(45, -76);
            SnyderProjection.getInstance().nativeToPYXIS(ll, i, 16);
            Trace.info("i=" + i.toString());

            // Notifier and observer
            PYXCamera_SPtr cam = PYXCamera.create();
            Trace.info("creating CSharpObserver");

            CSharpObserver csobs = new CSharpObserver();
            Trace.info("attaching CSharpObserver");
            cam.attach(csobs);

            Trace.info("calling method (should notify)");
            cam.setZoom(40);

            // Command manager and commands
            CommandManager cmdMgr = CommandManager.getInstance();
            NavigateCommand_SPtr cmd = NavigateCommand.create(cam);
            cam.setZoom(50);
            cmd.captureEndState();
            cmdMgr.recordCommand(navigation_model.DynamicPointerCast_Command(cmd));
            cmdMgr.undo();
            double fViewAngle = cam.getState().getViewAngle();
            Trace.info("fViewAngle undo should be 40 and is " + fViewAngle);
            cmdMgr.redo();
            fViewAngle = cam.getState().getViewAngle();
            Trace.info("fViewAngle redo should be 50 and is " + fViewAngle);

            Trace.info("detaching CSharpObserver");
            cam.detach(csobs);

            // Camera state and coordinates
            PYXCoord3DDouble coord = cam.getState().getPosition();
            Trace.info("position=" + coord.x() + "," + coord.y() + "," + coord.z());

            // Clear trace callback
            Trace.setTraceCallback(null);
            Trace.info("We shouldn't see this trace in C# Console, only in the trace.log.");

            Trace.time("Done app run");

            // Destroy libs
            NavigationModelInstance.destroy();
            PYXLibInstance.destroy();
        }
    }
}