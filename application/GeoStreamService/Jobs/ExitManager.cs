/******************************************************************************
ExitManager.cs

begin		: February 9, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System.Collections.Generic;

namespace GeoStreamService.Jobs
{
    /// <summary>
    /// Helper class to JobManager to synchronize the exiting of all jobs and job managers.
    /// </summary>
    internal static class ExitManager
    {
        /// <summary>
        /// Set by the GeoWeb Stream Server when it is ready to exit;
        /// running jobs monitor this to exit gracefully.
        /// </summary>
        public static bool ShouldExit
        {
            get
            {
                return s_shouldExit;
            }
            set
            {
                if (s_shouldExit != value)
                {
                    s_shouldExit = value;

                    if (s_shouldExit)
                    {
                        //ask all objects to stop
                        foreach (var objectToStop in s_objectsToStop)
                        {
                            objectToStop.Stop();
                        }

                        // pause the calling thread until all jobs and job managers are done
                        foreach (var objectToStop in s_objectsToStop)
                        {
                            objectToStop.Finished.Wait();
                        }
                    }
                }
            }
        }

        private static bool s_shouldExit = false;

        private static List<IJobManager> s_objectsToStop = new List<IJobManager>();

        internal static void AddObjectToStop(IJobManager objectToStop)
        {
            if (!s_objectsToStop.Contains(objectToStop))
            {
                s_objectsToStop.Add(objectToStop);
            }
        }
    }
}