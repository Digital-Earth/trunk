using System;
using System.Text;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    [TestFixture]
    public class DynamicListTests
    {
        TraceTool Trace = new TraceTool(false);

        class TestClass
        {
            public int data;
            public TestClass(int value)
            {
                data = value;
            }
        }

        [Test]
        public void TestConstruction()
        {
            DynamicList<int> aList = new DynamicList<int>();
        }

        [Test]
        public void TestAddition()
        {
            DynamicList<int> aList = new DynamicList<int>();
            aList.Add(1);
            aList.Add(3);
            aList.Add(7);
            aList.Add(3);
            NUnit.Framework.Assert.IsTrue(aList.Count == 4);
            NUnit.Framework.Assert.IsTrue(aList[1] == 3);

            aList.RemoveAll(delegate(int element) { return element == 3; });
            NUnit.Framework.Assert.IsTrue(aList.Count == 2);
            NUnit.Framework.Assert.IsTrue(aList[0] == 1);
            NUnit.Framework.Assert.IsTrue(aList[1] == 7);
        }

        [Test]
        public void TestEvents()
        {
            bool shouldAdd = true;
            int numberOfAddedEvents = 0;

            EventHandler<DynamicList<int>.AddingElementEventArgs> handleAdding =
                delegate(object sender, DynamicList<int>.AddingElementEventArgs args)
                {
                    args.AllowAddition = shouldAdd;
                };

            EventHandler<DynamicList<int>.ElementEventArgs> handleAdded =
                delegate(object sender, DynamicList<int>.ElementEventArgs args)
                {
                    numberOfAddedEvents++;
                    NUnit.Framework.Assert.IsTrue(
                        args.ElementCount == numberOfAddedEvents);
                };

            EventHandler<DynamicList<int>.ElementEventArgs> handleErroneousRemove =
                delegate(object sender, DynamicList<int>.ElementEventArgs args)
                {
                    // We're not doing any removing here, so this shouldn't get triggered.
                    NUnit.Framework.Assert.Fail();
                };

            DynamicList<int> aList = new DynamicList<int>();
            aList.AddingElement += handleAdding;
            aList.AddedElement += handleAdded;
            aList.RemovedElement += handleErroneousRemove;

            aList.Add(1);
            shouldAdd = false;
            aList.Add(3);
            shouldAdd = true;
            aList.Add(7);

            NUnit.Framework.Assert.AreEqual(2, aList.Count);
            NUnit.Framework.Assert.AreEqual(aList.Count, numberOfAddedEvents);
            NUnit.Framework.Assert.AreEqual(7, aList[1]);

            // Remove each element, and ensure that the proper events are called.
            {
                int numberOfRemovedEventsLeft = aList.Count;

                EventHandler<DynamicList<int>.ElementEventArgs> handleRemoved =
                    delegate(object sender, DynamicList<int>.ElementEventArgs args)
                    {
                        --numberOfRemovedEventsLeft;
                        NUnit.Framework.Assert.AreEqual(
                            args.ElementCount,
                            numberOfRemovedEventsLeft);
                    };

                aList.RemovedElement -= handleErroneousRemove;
                aList.RemovedElement += handleRemoved;

                for (int count = aList.Count; 0 < count; )
                {
                    aList.Remove(aList[--count]);
                    NUnit.Framework.Assert.AreEqual(count, aList.Count);
                }
                NUnit.Framework.Assert.AreEqual(0, aList.Count);
                NUnit.Framework.Assert.AreEqual(0, numberOfRemovedEventsLeft);
            }
        }

        [Test]
        public void TestThreadSafeIteration()
        {
            DynamicList<TestClass> myDynamicList = new DynamicList<TestClass>();

            System.Threading.ThreadPool.QueueUserWorkItem(delegate(Object unused)
            {
                for (int count = 1; count < 100; ++count)
                {
                    int newValue = TestData.GenerateInt(10000);
                    Trace.WriteLine("Adding {0}.", newValue);
                    myDynamicList.Add(new TestClass(newValue));
                }
            });

            System.Threading.ThreadPool.QueueUserWorkItem(delegate(Object unused)
            {
                for (int iteration = 0; iteration < 10; ++iteration)
                {
                    StringBuilder iterationText = new StringBuilder();
                    iterationText.AppendFormat("Iteration #{0} contains about {1} items. ",
                        iteration, myDynamicList.Count);
                    int itemNumber = 0;
                    foreach (TestClass item in myDynamicList)
                    {
                        iterationText.AppendFormat("{0}{1} ",
                            ((itemNumber > 0) ? ", " : ""),
                            item.data);
                        itemNumber++;
                    }
                    Trace.WriteLine(iterationText.ToString());
                    System.Threading.Thread.Sleep(1);
                }
            });

            System.Threading.ThreadPool.QueueUserWorkItem(delegate(Object unused)
            {
                for (int iteration = 0; iteration < 10; ++iteration)
                {
                    StringBuilder iterationText = new StringBuilder();
                    iterationText.AppendFormat("Second Iteration #{0} contains about {1} items. ",
                        iteration, myDynamicList.Count);
                    int itemNumber = 0;
                    foreach (TestClass item in myDynamicList)
                    {
                        iterationText.AppendFormat("{0}{1} ",
                            ((itemNumber > 0) ? ", " : ""),
                            item.data);
                        itemNumber++;
                    }
                    Trace.WriteLine(iterationText.ToString());
                    System.Threading.Thread.Sleep(1);
                }
            });
        }

        // TODO: write more unit tests that test the thread saftey of the dynamic list.
    }
}