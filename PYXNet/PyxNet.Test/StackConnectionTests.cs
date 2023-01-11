namespace PyxNet.Test
{
    public class StackConnectionTests: TestStackConnection 
    {
        [NUnit.Framework.Test]
        public void StackConnectionList()
        {
            System.Collections.Generic.List<StackConnection> myList =
                new System.Collections.Generic.List<StackConnection>();
            myList.Add(m_one);
            NUnit.Framework.Assert.AreEqual(myList.Count, 1);
            NUnit.Framework.Assert.Contains(m_one, myList,
                "An item that was just added should be in the list.");
            NUnit.Framework.Assert.IsFalse( myList.Remove(m_two), 
                "Should not be able to remove an item that isn't in the list");
            NUnit.Framework.Assert.IsTrue(myList.Remove(m_one),
                "Should be able to remove an item that is in the list.");
        }
    }
}