using ApplicationUtility;
using System.Threading.Tasks;
using Pyxis.Core.IO;

namespace Pyxis.IO.Import
{
    /// <summary>
    /// Helper class used to fix common errors when importing pipelines.
    /// this class would use IImportSettingProvider if extra setting are required to fix the process error
    /// </summary>
    internal static class ProcessErrorFixer
    {
        public static Task<IProcess_SPtr> Fix(IProcess_SPtr process,
            IImportSettingProvider settingsProvider)
        {
            if (process.getInitState() == IProcess.eInitStatus.knFailedToInit)
            {
                var errorProc = PipeUtils.findFirstError(process);

                //allow the process error fixer to fix the first error.
                return ProcessErrorFixer.Fix(process, errorProc, settingsProvider);
            }

            return Task.FromResult(process);
        }


        public static Task<IProcess_SPtr> Fix(
            IProcess_SPtr process,
            IProcess_SPtr errorProcess,
            IImportSettingProvider settingsProvider
            )
        {
            var error = errorProcess.getInitError();
            var errorId = error.getErrorID();

            switch (errorId)
            {
                case PYXCOMFactory.WellKnownProcessInitError.MissingSRSErrorID:
                    return FixMissingSRS(process, errorProcess, settingsProvider);

                case PYXCOMFactory.WellKnownProcessInitError.MissingUserCredentialsErrorID:
                    return FixMissingUserCredentials(process, errorProcess, settingsProvider);

                default:
                    return null;
            }
        }

        private static Task<IProcess_SPtr> FixMissingSRS(
            IProcess_SPtr process,
            IProcess_SPtr errorProcess,
            IImportSettingProvider settingsProvider
            )
        {
            var srsTask = settingsProvider.ProvideSetting(typeof(SRSImportSetting), null);

            if (srsTask == null)
            {
                return null;
            }

            return srsTask.ContinueWith(t =>
            {
                var srsSetting = t.Result as SRSImportSetting;

                if (srsSetting == null)
                {
                    return null;
                }

                var srsProcess = srsSetting.SRS.CreateSRSProcess();
                var srsProcessIID = PYXCOMFactory.WellKnownInterfaces.ISRS.ToString();

                var cloneMap = new Process_Process_Map();

                // modify a clone of the pipeline from the process 
                // missing the SRS up and get a reference to the root 
                // process of the modified pipeline
                var newRootProcess = PipeUtils.modifyPipeline(process, errorProcess, cloneMap);
                
                // add the SRS process to the pipeline
                var vecParams = cloneMap.get(errorProcess).getParameters();

                foreach (Parameter_SPtr param in vecParams)
                {
                    if (param.getSpec().getInterface().ToString() == srsProcessIID)
                    {
                        param.addValue(srsProcess);
                        break;
                    }
                }

                return newRootProcess;
            });
        }

        private static Task<IProcess_SPtr> FixMissingUserCredentials(
            IProcess_SPtr process,
            IProcess_SPtr errorProcess,
            IImportSettingProvider settingsProvider
            )
        {
            /*
            if (settingsProvider == null)
            {
                return null;
            }

            var task = settingsProvider.ProvideSetting(
                    typeof(NetworkCredentialImportSetting),
                    new NetworkCredentialImportSettingArgs
                    {
                        // TODO: pass the url as a parameter
                    }
                );
            if (task == null)
            {
                return null;
            }

            NetworkCredential credentials;
            task.ContinueWith((t) =>
            {
                var networkCredentialSetting = t.Result as NetworkCredentialImportSetting;
                if (networkCredentialSetting != null)
                {
                    credentials = networkCredentialSetting.Credentials;
                }
            }).
            Wait();
            */

            //TODO: not implemented yet
            return null;
        }
    }
}
