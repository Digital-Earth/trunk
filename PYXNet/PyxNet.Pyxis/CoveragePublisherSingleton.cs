/******************************************************************************
CoveragePublisherSingleton.cs

begin      : 07/09/2007 12:58:51 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Pyxis
{
    /// <summary>
    /// A singleton MultiPulisher.
    /// </summary>
    public static class PublisherSingleton
    {
        private static MultiPublisher s_publisher;

        /// <summary>
        /// Get the Publisher.
        /// </summary>
        public static MultiPublisher Publisher
        {
            get
            {
                if (s_publisher == null)
                {
                    s_publisher = new MultiPublisher(StackSingleton.Stack);
                }
                return s_publisher;
            }
        }
    }
}
