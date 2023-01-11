using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Core;
using Pyxis.IO.Import;

namespace Pyxis.WorldView.Studio.Layers.Html
{
    /// <summary>
    /// JsSettingsProvider is an helper class to allow the embedded browser JS to handle
    /// IImportSettingProvider.ProvideSettings callbacks
    /// </summary>
    public class JsSettingsProvider
    {
        private class DeferredImportAction : IImportSettingProvider
        {
            private JsSettingsProvider Provider { get; set; }
            public ImportGeoSourceProgress ImportAction { get; private set; }
            private Dictionary<Type, TaskCompletionSource<IImportSetting>> DeferredSettings { get; set; }
            
            private object m_lock = new object();            

            public DeferredImportAction(JsSettingsProvider provider)
            {
                Provider = provider;
                DeferredSettings = new Dictionary<Type, TaskCompletionSource<IImportSetting>>();                
            }

            public void Start(DataSet dataSet)
            {
                ImportAction = Provider.Engine.BeginImport(dataSet, this);
            }

            public void Cancel()
            {
                Provider.RemoveImportAction(this.ImportAction.DataSet);
            }

            public Task<IImportSetting> ProvideSetting(Type requiredSetting, ProvideImportSettingArgs args)
            {
                lock (m_lock)
                {
                
                    if (!DeferredSettings.ContainsKey(requiredSetting))
                    {
                        DeferredSettings[requiredSetting] = new TaskCompletionSource<IImportSetting>();
                        Provider.InvokeSettingRequested(ImportAction.DataSet, requiredSetting, args);
                    }

                    return DeferredSettings[requiredSetting].Task;
                }
            }

            public void SetImportSettingFailed(Type requestedSetting, Exception error)
            {
                lock (m_lock)
                {
                    if (!DeferredSettings.ContainsKey(requestedSetting))
                    {
                        return;
                    }

                    DeferredSettings[requestedSetting].SetException(error);
                    DeferredSettings.Remove(requestedSetting);
                }
            }

            public void SetImportSetting(IImportSetting setting)
            {
                lock (m_lock)
                {                    
                    var type = setting.GetType();

                    if (!DeferredSettings.ContainsKey(type))
                    {
                        return;
                    }

                    DeferredSettings[type].SetResult(setting);
                    DeferredSettings.Remove(type);
                }
            }
        }

        public class OnImportSettingRequestEventArgs : EventArgs
        {
            public Type RequiredSetting { get; set; }
            public ProvideImportSettingArgs Args { get; set; }
            public DataSet DataSet { get; set; }
        }


        private Dictionary<string, DeferredImportAction> m_importActions = new Dictionary<string, DeferredImportAction>();
        private object m_importActionsLock = new object();

        private Engine Engine { get; set; }

        /// <summary>
        /// SettingRequested Event is invoked every time an ImportSetting is requested
        /// </summary>
        public event EventHandler<OnImportSettingRequestEventArgs> SettingRequested;

        public JsSettingsProvider(Engine engine)
        {
            Engine = engine;
        }

        /// <summary>
        /// Create an ImportGeoSourceProgress for a given url.
        /// </summary>
        /// <param name="dataSet">The data set to import</param>
        /// <returns>Pyxis.Core.IO.Import.ImportGeoSourceProgress object.</returns>
        public ImportGeoSourceProgress GetImportAction(DataSet dataSet)
        {
            lock (m_importActionsLock)
            {
                DeferredImportAction deferredAction;
                if (!m_importActions.TryGetValue(dataSet.ToString(), out deferredAction))
                {
                    deferredAction = new DeferredImportAction(this);
                    m_importActions[dataSet.ToString()] = deferredAction;
                    deferredAction.Start(dataSet);
                    deferredAction.ImportAction.Task.ContinueWith((t) => RemoveImportAction(dataSet));
                }

                return deferredAction.ImportAction;
            }
        }

        /// <summary>
        /// Return all active import actions.
        /// </summary>
        /// <returns>List of Pyxis.Core.IO.Import.ImportGeoSourceProgress.</returns>
        public List<ImportGeoSourceProgress> GetActiveImportActions()
        {
            lock (m_importActionsLock)
            {
                return m_importActions.Values.Select(x => x.ImportAction).ToList();
            }
        }

        /// <summary>
        /// Cancel an on going import action
        /// </summary>
        /// <param name="dataSet">DataSet for which to cancel the import.</param>        
        public void RemoveImportAction(DataSet dataSet)
        {
            lock (m_importActionsLock)
            {
                m_importActions.Remove(dataSet.ToString());
            }
        }

        /// <summary>
        /// Notify that a requested import setting cannot be provided.
        /// </summary>
        /// <param name="dataSet">Data set to identify the import action to update.</param>
        /// <param name="requestedSetting">Type of requested import settings.</param>
        /// <param name="error">Exception to use to notify reason.</param>
        public void SetImportSettingFailed(DataSet dataSet, Type requestedSetting, Exception error)
        {
            lock (m_importActionsLock)
            {
                DeferredImportAction deferredAction;
                if (!m_importActions.TryGetValue(dataSet.ToString(), out deferredAction))
                {
                    return;
                }

                deferredAction.SetImportSettingFailed(requestedSetting, error);                
            }
        }

        /// <summary>
        /// Provide import setting to a import action.
        /// </summary>
        /// <param name="dataSet">Data set to identify the import action to update.</param>
        /// <param name="setting">The provided import setting object.</param>
        public void SetImportSetting(DataSet dataSet, IImportSetting setting)
        {
            lock (m_importActionsLock)
            {
                DeferredImportAction deferredAction;
                if (!m_importActions.TryGetValue(dataSet.ToString(), out deferredAction))
                {
                    return;
                }

                deferredAction.SetImportSetting(setting);                
            }
        }


        private void InvokeSettingRequested(DataSet dataSet, Type requiredSetting, ProvideImportSettingArgs args)
        {
            var handler = SettingRequested;

            if (handler != null)
            {
                handler.Invoke(this, new OnImportSettingRequestEventArgs()
                {
                    DataSet = dataSet,
                    Args = args,
                    RequiredSetting = requiredSetting
                });
            }
        }
    }
}
