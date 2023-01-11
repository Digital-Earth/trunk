using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace ApplicationUtility.Visualization
{
    /// <summary>
    /// Helper class that will allow background threads to invoke methods on the UI thread.
    /// </summary>
    public static class UIThread
    {
        /// <summary>
        /// This need to point to the ApplicationForm class or any other main form of the application.
        /// </summary>
        public static Form Parent { get; set; }

        public static void BeginInvoke(Delegate aDelegate)
        {
            if (Parent == null)
            {
                throw new Exception("MainControl hasn't been set.");
            }

            Parent.BeginInvoke(aDelegate);
        }

        public static void Invoke(Delegate aDelegate)
        {
            if (Parent == null)
            {
                throw new Exception("MainControl hasn't been set.");
            }

            Parent.Invoke(aDelegate);
        }

        public static bool InvokeRequired
        {
            get
            {
                if (Parent == null)
                {
                    throw new Exception("MainControl hasn't been set.");
                }

                return Parent.InvokeRequired;
            }
        }        
    }
}
