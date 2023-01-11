using System;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for EventHelper
    /// </summary>
    [TestFixture]
    public class EventHelperTests
    {
        [Test]
        public void DirectUseOfEventHelper()
        {
            // Note: We don't usually use this as a local variable.  It's normally used as a field.
            EventHelper<EventArgs> myEventHelper = new EventHelper<EventArgs>();

            ThreadSafeInt callCount = new ThreadSafeInt();

            EventHandler<EventArgs> sampleHandler = delegate( object o, EventArgs e)
            {
                ++callCount;
            };

            for (int i = 0; i < 100; ++i)
            {
                myEventHelper.Add(sampleHandler);
            }

            myEventHelper.Invoke(this, new EventArgs());
            Assert.AreEqual(callCount.Value, 100);

            for (int j = 0; j < 99; ++j)
            {
                myEventHelper.Remove(sampleHandler);
            }
            myEventHelper.Invoke(this, new EventArgs());
            Assert.AreEqual(callCount.Value, 101);
            myEventHelper.Remove(sampleHandler);
            myEventHelper.Remove(sampleHandler);
        }
    }
}