/******************************************************************************
Context.cs

begin      : October 29, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

using Pyxis.Utilities;

namespace PyxNet
{
    /// <summary>
    /// Context for storing PyxNet properties.
    /// By default, these properties are stored as system properties.
    /// The ASP applications need to override the storage paradigm.
    /// </summary>
    public class Context : Pyxis.Utilities.ContextRepository
    {
        /// <summary>
        /// Global PyxNet Context
        /// </summary>
        static private Context m_ctx = new Context();

        public Context()
        {
            //--
            //-- when we create a new context, it becomes the global one.
            //--
            HostCtx = this;
        }

        /// <summary>
        /// Gets or sets the Global PyxNet Context.
        /// </summary>
        /// <value>Global PyxNet Context.</value>
        static private Context HostCtx
        {
            get { return m_ctx; }
            set { m_ctx = value; }
        }

        /// <summary>
        /// Resets this instance.
        /// </summary>
        static public void Reset()
        {
            HostCtx.Init();
        }

        /// <summary>
        /// Loads this instance.
        /// </summary>
        static public void Load()
        {
            HostCtx.LoadContext();
        }

        /// <summary>
        /// Saves this instance.
        /// </summary>
        static public void Save()
        {
            HostCtx.SaveContext();
        }

        #region Context Properties

        static public string StackName
        {
            get { return ((string)(HostCtx["StackName"])); }
            set { HostCtx["StackName"] = value; }
        }

        static public string PrivateKey
        {
            get { return ((string)(HostCtx["PrivateKey"])); }
            set { HostCtx["PrivateKey"] =  value; }
        }

        static public string NodeID
        {
            get { return ((string)(HostCtx["NodeID"])); }
            set { HostCtx["NodeID"] = value; }
        }

        static public string ExternalAddresses
        {
            get { return ((string)(HostCtx["ExternalAddresses"])); }
            set { HostCtx["ExternalAddresses"] = value; }
        }

        static public int ConnectionRetryWaitMilliseconds
        {
            get { return ((int)(HostCtx["ConnectionRetryWaitMilliseconds"])); }
            set { HostCtx["ConnectionRetryWaitMilliseconds"] = value; }
        }

        #endregion

        /// <summary>
        /// default context loader reads values from system properties.
        /// </summary>
        protected override void OnLoad()
        {
            StackName = Properties.Settings.Default.StackName;
            PrivateKey = Properties.Settings.Default.PrivateKey;
            NodeID = Properties.Settings.Default.NodeID;
            ExternalAddresses = Properties.Settings.Default.ExternalAddresses;
            ConnectionRetryWaitMilliseconds = Properties.Settings.Default.ConnectionRetryWaitMilliseconds;
        }

        /// <summary>
        /// default context save writes values to system properties.
        /// </summary>
        protected override void OnSave()
        {
            Properties.Settings.Default.StackName = StackName;
            Properties.Settings.Default.PrivateKey = PrivateKey;
            Properties.Settings.Default.NodeID = NodeID;
            Properties.Settings.Default.ExternalAddresses = ExternalAddresses;
            Properties.Settings.Default.ConnectionRetryWaitMilliseconds = ConnectionRetryWaitMilliseconds;

            Properties.Settings.Default.Save();
        }

        /// <summary>
        /// Upgrades this instance.
        /// </summary>
        public static void Upgrade()
        {
            Properties.Settings.Default.Upgrade();
        }
    }    
}
