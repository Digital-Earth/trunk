using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    [TestFixture]
    class ArgsParserTests
    {
        [Test]
        public void TestPrefix()
        {
            var arguments = new[]{ "c", "-c", "---c", "-c=1"," -c" };
            var count = 0;
            var extra = ArgsParser.Parse(arguments, new ArgsParser.Option("c", (value) => count++));
            Assert.AreEqual(3, count);
            Assert.AreEqual(extra, new[] { "c"," -c" }); // NOTE: " -c" starts with space.
        }

        [Test]
        public void TestMultiname()
        {
            var arguments = new[] { "c" /* not prefix - not like "-c" */ , "-c", "-cat", "-d", "-mouse", "-lion", "-dog=1", "-big-dog", "-d=1", "-car" };
            var count1 = 0;
            var count2 = 0;
            var extra = ArgsParser.Parse(arguments, 
                new ArgsParser.Option("c|cat|lion", (value) => count1++),
                new ArgsParser.Option("d|dog|big-dog", (value) => count2++));                
            Assert.AreEqual(3, count1);
            Assert.AreEqual(4, count2);
            Assert.AreEqual(extra, new[] { "c", "-mouse", "-car" });
        }

        [Test]
        public void TestValue()
        {
            var arguments = new[] { "-empty", "-number=1", "-string=hello", "-number2:2", "-string2:hi" };
            var extra = ArgsParser.Parse(arguments,
                new ArgsParser.Option("empty", (value) => Assert.AreEqual("", value)),
                new ArgsParser.Option("number", (value) => Assert.AreEqual("1", value)),
                new ArgsParser.Option("string", (value) => Assert.AreEqual("hello", value)),
                new ArgsParser.Option("number2", (value) => Assert.AreEqual("2", value)),
                new ArgsParser.Option("string2", (value) => Assert.AreEqual("hi", value)));

            Assert.AreEqual(extra, new string[] { });
        }
    }
}