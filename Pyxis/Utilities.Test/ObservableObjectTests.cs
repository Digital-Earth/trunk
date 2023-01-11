using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for ObservableObject
    /// </summary>
    [TestFixture]
    public class ObservableObjectTests
    {
        [Test]
        public void Integer()
        {
            bool callbackHappened = false;

            ObservableObject<int> myObject = new ObservableObject<int>(13);
            myObject.Changed +=
                delegate(object sender, ChangedEventArgs<int> e)
                {
                    Assert.AreEqual(42, e.NewValue);
                    callbackHappened = true;
                };
            myObject.Value = 42;
            Assert.IsTrue(callbackHappened);
        }
    }
}