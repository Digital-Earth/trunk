using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for ProgressData
    /// </summary>
    [TestFixture]
    public class ProgressDataTests
    {
        [Test]
        public void CommonUsage()
        {
            ProgressData item = new ProgressData
            {
                Title = "Test item",
                FinalValue = 10,
                Units = "items"
            };

            string firstString = item.ToString();
            ++item.CurrentValue.Value;
            string nextString = item.ToString();

            bool called = false;
            item.CurrentValue.Changed += delegate(object o, ChangedEventArgs<int> arg)
            {
                called = true;
            };
            Assert.IsFalse(called, "CurrentValue.Changed was called too soon.");
            ++item.CurrentValue.Value;
            Assert.IsTrue(called, "CurrentValue.Changed should have been called.");
        }
    }
}