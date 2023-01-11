using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace LeakClarification
{
    class RefencedByAnEventHandler
    {
        public RefencedByAnEventHandler() {}

        private byte[] bigThing = new byte[10000];

        public int counter = 0;
    }

    /// <summary>
    /// This leak is where an object that is referenced in an event handler stays around because the event handler
    /// is not unhooked properly.
    /// </summary>
    public class ClassLeak1
    {
        public ClassLeak1()
        {
            Make1000Objects();
        }

        public EventHandler MyEvent;

        public void Make1000Objects()
        {
            for (int count = 0; count < 1000; ++count)
            {
                RefencedByAnEventHandler a = new RefencedByAnEventHandler();

                MyEvent +=
                    delegate(object sender, EventArgs e)
                    {
                        // it is this reference to variable a that will keep it in memory.
                        ++a.counter;
                    };

                ++a.counter;

                // this does NOT actually unhook the event, and so as long as the 
                // object ClassLeak1 hangs around, so will the object "a" hang around and eat up
                // all the memory.
                MyEvent -=
                    delegate(object sender, EventArgs e)
                    {
                        ++a.counter;
                    };
            }
        }
    }


    /// <summary>
    /// This is the class above, but with the event handler properly unhooked so that the memory usage does not occur.
    /// </summary>
    public class ClassNoLeak1
    {
        public ClassNoLeak1()
        {
            Make1000Objects();
        }

        public EventHandler MyEvent;

        public void Make1000Objects()
        {
            for (int count = 0; count < 1000; ++count)
            {
                RefencedByAnEventHandler a = new RefencedByAnEventHandler();
                EventHandler handler =
                    delegate(object sender, EventArgs e)
                    {
                        // it is this reference to variable a that will keep it in memory
                        // as long as the event is referenced.
                        ++a.counter;
                    };
                MyEvent += handler;

                ++a.counter;

                // This does unhook the event, and it will not matter about the life time of
                // object ClassNoLeak1  the object "a" will not hang around and eat up
                // all the memory.
                MyEvent -= handler;
            }
        }
    }


    /// <summary>
    /// This is the class above, but with the event handler properly unhooked so that the memory usage does not occur.
    /// </summary>
    public class ClassLeak2
    {
        public ClassLeak2()
        {
            Make1000Objects();
        }

        public EventHandler MyEvent;

        public void Make1000Objects()
        {
            for (int count = 0; count < 1000; ++count)
            {
                RefencedByAnEventHandler a = new RefencedByAnEventHandler();
                EventHandler handler =
                    delegate(object sender, EventArgs e)
                    {
                        // it is this reference to variable a that will keep it in memory
                        // as long as the event is referenced.
                        ++a.counter;
                    };
                MyEvent += handler;

                ++a.counter;

                // we don't unhook the event so a reference to "a" is still around and we eat up memory.
                //MyEvent -= handler;
            }
        }
    }


    /// <summary>
    /// This is the class above, but with the event handler properly unhooked so that the memory usage does not occur.
    /// </summary>
    public class ClassNoLeak2
    {
        public ClassNoLeak2()
        {
            Make1000Objects();
        }

        public EventHandler MyEvent;

        public void Make1000Objects()
        {
            for (int count = 0; count < 1000; ++count)
            {
                RefencedByAnEventHandler a = new RefencedByAnEventHandler();
                EventHandler handler =
                    delegate(object sender, EventArgs e)
                    {
                        int i = 0;
                        ++i;
                    };
                MyEvent += handler;

                ++a.counter;

                // we leave the event ahndler hooked up, but since object "a" is not referenced by the
                // event handler code, it gets garbage collected and doesn't eat memory.
                //MyEvent -= handler;
            }
        }
    }
}
