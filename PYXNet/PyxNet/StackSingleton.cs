/******************************************************************************
StackSingleton.cs

begin      : 04/04/2007 12:58:51 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet
{
    // TODO: When PyxNetStack is renamed to LocalNode, rename this to LocalNodeManager
    // and modify interface accordingly.
    /// <summary>
    /// A singleton PyxNetStack. 
    /// </summary>
    public static class StackSingleton
    {
        /// <summary>
        /// A lock that ensures that only one stack gets created.
        /// </summary>
        private static readonly Object s_stackLock = new Object();

        private static int s_initialPort = PyxNetStack.NodePort;

        /// <summary>
        /// The initial port to try to listen on.
        /// </summary>
        public static int InitialPort
        {
            get { return s_initialPort; }
            set { s_initialPort = value; }
        }

        private static int s_numberOfPortsToTry = PyxNetStack.DefaultNumberOfPortsToTry;

        /// <summary>
        /// The number of ports to try and open a listening socket on.
        /// This only kicks in if we can not listen on the port we want,
        /// and have to try subsequent ports.
        /// </summary>
        public static int NumberOfPortsToTry
        {
            get { return s_numberOfPortsToTry; }
            set { s_numberOfPortsToTry = value; }
        }

        /// <summary>
        /// The lazily-constructed stack.
        /// </summary>
        /// <remarks>
        /// The <code>volatile</code> keyword is recommended by Microsoft.
        /// </remarks>
        /// <see cref="http://msdn.microsoft.com/en-us/library/ms998558.aspx"/>
        private static volatile PyxNetStack s_stack;

        private static IPyxNetStackConfiguration s_configuration = null;
        public static IPyxNetStackConfiguration Configuration
        {
            get
            {
                if (s_stack != null)
                {
                    return s_stack.Configuration;
                }
                return s_configuration;
            }
            set
            {
                if (s_stack != null)
                {
                    throw new Exception("Can't set configuration after stack has been created");
                }
                s_configuration = value;
            }
        }

        /// <summary>
        /// Get the stack.
        /// </summary>
        public static PyxNetStack Stack
        {
            get
            {
                if (s_stack == null)
                {
                    lock (s_stackLock)
                    {
                        if (s_stack == null)
                        {
                            if (s_configuration == null)
                            {
                                s_stack = new PyxNetStack(InitialPort, NumberOfPortsToTry);
                            }
                            else
                            {
                                s_stack = new PyxNetStack(InitialPort, NumberOfPortsToTry,s_configuration);
                            }
                        }
                    }
                }
                return s_stack;
            }
        }
    }
}
