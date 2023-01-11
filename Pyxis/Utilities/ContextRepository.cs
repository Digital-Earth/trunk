/******************************************************************************
ContextRepository.cs

begin      : October 29, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Context repository is an object to load and save context properties.
    /// Class is designed to allow one to override the default load and save mechanisms.
    /// 
    /// Basic problem solved by this class is that some of our assemblies were reading
    /// persistent properties from user property settings.  This approach does not work
    /// with deployed ASP.Net applications that do not have access to these files.
    /// 
    /// If one reads and writes the property files through one of these context reposoitories,
    /// then applications using the assembies can redefine the load and save routines
    /// to accommondate the deployment environment.
    /// </summary>
    public abstract class ContextRepository
    {
        /// <summary>
        /// abstract declarations for the default loading and saving mechanisms
        /// each derived context must define these.
        /// </summary>
        protected abstract void OnLoad();
        protected abstract void OnSave();

        /// <summary>
        /// Delegate type for loading and saving callback procedures.
        /// </summary>
        public delegate void LoadSaveCallbackProc();

        /// <summary>
        /// Object for storing the context properties.
        /// </summary>
        private Dictionary<string, object> m_items = new Dictionary<string, object>();

        /// <summary>
        /// Has the context been loaded?
        /// Used when accessing a context property to determine if anything
        /// has been loaded yet.
        /// </summary>
        private bool HasLoaded
        {
            get;
            set;
        }

        /// <summary>
        /// Callback function called when loading context via a secondary
        /// or auxilary loading mechanism.
        /// </summary>
        public LoadSaveCallbackProc OnHostLoad
        {
            get;
            set;
        }

        /// <summary>
        /// callback function called when saving context via a seconary
        /// or auxilary saving mechanism.
        /// </summary>
        public LoadSaveCallbackProc OnHostSave
        {
            get;
            set;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ContextRepository"/> class.
        /// </summary>
        protected ContextRepository()
        {
            Init();
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ContextRepository"/> class,
        /// with callbacks to override default load/save mechanisms.
        /// </summary>
        /// <param name="loadProc">OnLoad callback procedure.</param>
        /// <param name="saveProc">OnSave callback procedure.</param>
        public ContextRepository(LoadSaveCallbackProc loadProc, LoadSaveCallbackProc saveProc)
        {
            Init();
            OnHostLoad = loadProc;
            OnHostSave = saveProc;
        }

        /// <summary>
        /// Inits this instance, can be used for resets.
        /// </summary>
        protected void Init()
        {
            HasLoaded = false;
            OnHostLoad = null;
            OnHostSave = null;

            m_items.Clear();
        }

        /// <summary>
        /// Loads the context, called from derived context when loading.
        /// </summary>
        protected void LoadContext()
        {
            lock (m_items)
            {
                HasLoaded = true;
                if (OnHostLoad != null)
                {
                    OnHostLoad();
                }
                else
                {
                    OnLoad();
                }
            }
        }

        /// <summary>
        /// Saves the context, called from derived context when saving.
        /// </summary>
        protected void SaveContext()
        {
            lock (m_items)
            {
                if (OnHostSave != null)
                {
                    OnHostSave();
                }
                else
                {
                    OnSave();
                }
            }
        }

        /// <summary>
        /// Gets or sets the <see cref="System.Object"/> with the specified key.
        /// </summary>
        /// <value>Context property object.</value>
        protected object this[string key]
        {
            get 
            {
                lock (m_items)
                {
                    //--
                    //-- load context values if not done so already.
                    //--
                    if (HasLoaded == false)
                    {
                        LoadContext();
                    }

                    //--
                    //-- throw error if key does not reference object in context
                    //--
                    if (m_items.ContainsKey(key) == false)
                    {
                        string msg = string.Format("Key[{0}] is not contained in context.");
                        throw new System.ArgumentException(msg,key);
                    }

                    return m_items[key];
                }
            }
            
            set 
            {
                lock (m_items)
                {
                    m_items[key] = value;
                }
            }
        }
    }
}
