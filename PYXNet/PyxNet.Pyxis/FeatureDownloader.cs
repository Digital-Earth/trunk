/******************************************************************************
FeatureDownloader.cs

begin      : 26/08/2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

#if !NO_LIBRARY
using Pyxis.Services.PipelineLibrary.Repositories;
#endif

namespace PyxNet.Pyxis
{
    public class FeatureDownloader
    {
        /// <summary>
        /// Helper function for getting a remote process into the library.
        /// </summary>
        /// <param name="queryResults">A list of query results for the feature.</param>
        /// <returns>True if the remote process was imported.</returns>
        static public bool ImportRemoteProcess(List<QueryResult> queryResults)
        {
            if ((queryResults.Count == 0) ||
                !queryResults[0].ExtraInfo.StartsWith(PyxNet.Pyxis.FeatureDefinitionMessage.MessageID))
            {
                return false;
            }

            PyxNet.Pyxis.FeatureDefinitionMessage extraInfo = 
                new PyxNet.Pyxis.FeatureDefinitionMessage(queryResults[0].ExtraInfo);

            // get the .ppl from the coverageRequest and import the .ppl into the library.
            Vector_IProcess vecProcesses = PipeManager.importStr(extraInfo.PipelineDefinition);

            #if !NO_LIBRARY
            // set all the processes as not temporary
            PipelineRepository.Instance.SetIsTemporary(vecProcesses, false);
            
            // set the top of the process chain as NOT hidden
            PipelineRepository.Instance.SetIsHidden(extraInfo.ProcRef, false);
            PipelineRepository.Instance.SetIsPublished(extraInfo.ProcRef, true);
            #endif

            return true;
        }

    }
}
