using System;
using System.Collections.Generic;
using Pyxis.Core;

namespace PyxisCLI
{
    /// <summary>
    /// EngineCleanup is a utility class that can be used to clean up PYXLIB referneces before Pyxis.Engine.Stop is been called
    /// </summary>
    class EngineCleanup
    {
        private static readonly List<Action> s_callbacks = new List<Action>();
        private static readonly object s_lock = new object();

        public EngineCleanup(Action callback)
        {
            lock (s_lock)
            {
                s_callbacks.Add(callback);
            }
        }

        public static void AttachToEngine(Engine engine)
        {
            engine.BeforeStopping(() =>
            {
                lock (s_lock)
                {
                    s_callbacks.ForEach(action => action());
                }
            });
        }
    }
}
