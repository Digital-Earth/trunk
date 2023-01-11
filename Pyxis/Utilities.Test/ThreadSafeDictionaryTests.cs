using System;
using System.Collections.Generic;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    [TestFixture]
    public class ThreadSafeDictionaryTests
    {
        [Test]
        public void SimpleOperations()
        {
            ThreadSafeDictionary<int, string> dictionary = new ThreadSafeDictionary<int, string>();
            Assert.AreEqual(0, dictionary.Count, "Dictionary should be empty when constructed.");
            dictionary.Add(1, "Test");
            Assert.AreEqual(dictionary[1], "Test");
            Assert.AreEqual(1, dictionary.Count);
            dictionary[1] = "Second value";
            Assert.AreEqual(1, dictionary.Count);
            dictionary[2] = "Another value";
            Assert.AreEqual(2, dictionary.Count);
        }

        [Test]
        public void FiftyThreads()
        {
            const int totalThreads = 50;
            ThreadSafeDictionary< int, string> sharedDictionary = new ThreadSafeDictionary<int,string>();

            System.Threading.ManualResetEvent startingGun = new System.Threading.ManualResetEvent(false);

            ThreadSafeInt added = new ThreadSafeInt();
            ThreadSafeInt removed = new ThreadSafeInt();
            ThreadSafeInt found = new ThreadSafeInt();
            ThreadSafeInt liveThreads = new ThreadSafeInt(totalThreads * 3);

            for (int threadCount = 0; threadCount < totalThreads; ++threadCount)
            {
                // This thread creates an item (count, "count")
                System.Threading.ThreadPool.QueueUserWorkItem(
                    delegate( Object number)
                    {
                        int actualNumber = (int) number;
                        startingGun.WaitOne();
                        sharedDictionary.Add( actualNumber, actualNumber.ToString());
                        ++added;
                        --liveThreads;
                    }, threadCount);
                // This thread removes that item.  (It might not exist yet, but that's okay.)
                System.Threading.ThreadPool.QueueUserWorkItem(
                    delegate(Object number)
                    {
                        int actualNumber = (int)number;
                        startingGun.WaitOne();
                        if (sharedDictionary.Remove(actualNumber))
                        {
                            ++removed;
                        }
                        --liveThreads;
                    }, threadCount);
                // This thread accesses  the item, and access the list as well.
                System.Threading.ThreadPool.QueueUserWorkItem(
                    delegate(Object number)
                    {
                        int actualNumber = (int)number;
                        startingGun.WaitOne();
                        if (sharedDictionary.ContainsKey(actualNumber))
                        {
                            ++found;
                        }
                        foreach (var f in sharedDictionary)
                        {
                            string temp = f.ToString();
                        }
                        foreach (var f in (sharedDictionary as System.Collections.IEnumerable))
                        {
                            string temp = f.ToString();
                        }
                        foreach (var f in (sharedDictionary as IEnumerable<KeyValuePair<int, string>>))
                        {
                            string temp = f.ToString();
                        }
                        --liveThreads;
                    }, threadCount);
            }

            Assert.AreEqual(totalThreads * 3, liveThreads,
                "Didn't expect threads to start before signal.");

            // Now that all 150 threads are ready to go, let's fire the starting gun.
            startingGun.Set();

            // Wait until they've all finished.
            TimedTest.Verify(
                delegate() { return liveThreads.Value == 0; },
                TimeSpan.FromSeconds(5),
                "Thread pool did not complete all tasks in a reasonable amount of time.");

            // And finally, make sure our count is accurate.
            Assert.AreEqual(added.Value - removed.Value, sharedDictionary.Count);
        }
    }
}