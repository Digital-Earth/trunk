using System;
using System.Threading.Tasks;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    [TestFixture]
    public class SimpleTimeTests
    {
        [Test]
        public void TimerStartWithHandler()
        {
            var timeSpan = TimeSpan.FromMilliseconds(50);
            var start = DateTime.Now;                
            var end = DateTime.Now;

            var called = new TaskCompletionSource<bool>();
                
            SimpleTimer.Create(timeSpan, (s, e) =>
            {
                end = DateTime.Now;
                called.SetResult(true);
            });

            called.Task.Wait();
                
            Assert.Greater((end - start).TotalSeconds, timeSpan.TotalSeconds);
        }

        [Test]
        public void TimerDoesntStartAutomaticaly()
        {
            var timeSpan = TimeSpan.FromMilliseconds(50);
            var start = DateTime.Now;
            var end = DateTime.Now;
            var called = false;

            var calledTask = new TaskCompletionSource<bool>();

            var timer = SimpleTimer.Create(timeSpan);

            System.Threading.Thread.Sleep(timeSpan);
            Assert.AreEqual(false, called);

            System.Threading.Thread.Sleep(timeSpan);
            Assert.AreEqual(false, called);

            timer.Elapsed += (s, e) =>
            {
                end = DateTime.Now;
                called = true;
                calledTask.SetResult(true);
            };

            calledTask.Task.Wait();
                
            Assert.AreEqual(true, called);
        }
    }
}