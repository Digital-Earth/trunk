using System;
using System.Text;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    [TestFixture]
    public class WeakReferenceListTests
    {
        Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(false);

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
            WeakReferenceList<TestClass> aList = new WeakReferenceList<TestClass>();
        }

        [Test]
        public void TestAddition()
        {
            WeakReferenceList<TestClass> aList = new WeakReferenceList<TestClass>();
            TestClass test1 = new TestClass(1);
            TestClass test2 = new TestClass(2);
            TestClass test3 = new TestClass(3);
            aList.Add(test1);
            aList.Add(test2);
            aList.Add(test3);
            NUnit.Framework.Assert.AreEqual(3, aList.Count);
            NUnit.Framework.Assert.AreEqual(test1.data, aList[0].data);
            NUnit.Framework.Assert.AreEqual(test2.data, aList[1].data);
            NUnit.Framework.Assert.AreEqual(test3.data, aList[2].data);

            GC.KeepAlive(test1);
            GC.KeepAlive(test2);
            GC.KeepAlive(test3);
        }

        [Test]
        public void TestThreadSafeIteration()
        {
            WeakReferenceList<TestClass> myWeakReferenceList = new WeakReferenceList<TestClass>();

            System.Threading.ThreadPool.QueueUserWorkItem(delegate(Object unused)
            {
                for (int count = 1; count < 100; ++count)
                {
                    int newValue = TestData.GenerateInt(10000);
                    Trace.WriteLine("Adding {0}.", newValue);
                    myWeakReferenceList.Add(new TestClass(newValue));
                }
            });

            System.Threading.ThreadPool.QueueUserWorkItem(delegate(Object unused)
            {
                for (int iteration = 0; iteration < 10; ++iteration)
                {
                    StringBuilder iterationText = new StringBuilder();
                    iterationText.AppendFormat("Iteration #{0} contains about {1} items. ",
                        iteration, myWeakReferenceList.Count);
                    int itemNumber = 0;
                    myWeakReferenceList.ForEach(delegate(TestClass item)
                    {
                        iterationText.AppendFormat("{0}{1} ",
                            ((itemNumber > 0) ? ", " : ""),
                            item.data);
                        itemNumber++;
                    });
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
                        iteration, myWeakReferenceList.Count);
                    int itemNumber = 0;
                    myWeakReferenceList.ForEach(delegate(TestClass item)
                    {
                        iterationText.AppendFormat("{0}{1} ",
                            ((itemNumber > 0) ? ", " : ""),
                            item.data);
                        itemNumber++;
                    });
                    Trace.WriteLine(iterationText.ToString());
                    System.Threading.Thread.Sleep(1);
                }
            });
        }

        // TODO: write more unit tests that test the thread saftey of the weak reference list.
    }
}