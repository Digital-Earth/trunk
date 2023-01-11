/******************************************************************************
ViewPointProcessUtility.cs

begin      : August 26, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#define Airborne_Imaging_Demo

using System;
using System.Collections.Generic;
using System.Text;

using Pyxis.Services.PipelineLibrary.Domain;
using Pyxis.Services.PipelineLibrary.Repositories;

namespace ApplicationUtility
{
    // TODO[kabiraman]: Consider converting this into a non-static class that 
    // holds onto the ViewPointProcess.
    // TODO[kabiraman]: Consider separating into 2 classes for those 
    // methods that can remain static because they don't require knowledge of the ViewPointProcess.

    /// <summary>
    /// Provides methods to perform operations on a ViewPoint process.
    /// </summary>
    public static class ViewPointProcessUtility
    {
        // mutex
        private static object m_lockObject = new object();
        
        /// <summary>
        /// Adds the provided pipeline as an input to the ViewPoint process.
        /// </summary>
        /// <param name="viewPointProcess">The ViewPoint process.</param>
        /// <param name="procRef">The pipeline to add.</param>
        public static void AddPipeline(
            IProcess_SPtr viewPointProcess, 
            ProcRef procRef)
        {
            AddPipeline(viewPointProcess, PipeManager.getProcess(procRef));            
        }

        /// <summary>
        /// Adds the provided pipeline as an input to the ViewPoint process.
        /// </summary>
        /// <param name="viewPointProcess">The ViewPoint process.</param>
        /// <param name="process">The pipeline to add.</param>
        public static void AddPipeline(
            IProcess_SPtr viewPointProcess,
            IProcess_SPtr process)
        {
            lock (m_lockObject)
            {
                if (!ContainsPipeline(viewPointProcess, new ProcRef(process)))
                {
                    int parameter = 0;

                    if (IsOutputAFeatureCollection(process))
                    {
                        parameter = 1;
                    }

                    viewPointProcess.getParameter(parameter).addValue(process);                        
                }
            }
        }

        /// <summary>
        /// Removes the provided pipeline, if it exists, from the ViewPoint process.
        /// </summary>
        /// <param name="viewPointProcess">The ViewPoint process.</param>
        /// <param name="process">The pipeline to remove.</param>
        public static void RemovePipeline(
            IProcess_SPtr viewPointProcess,
            IProcess_SPtr process)
        {
            lock (m_lockObject)
            {
                int parameter = 0;

                if (IsOutputAFeatureCollection(process))
                {
                    parameter = 1;
                }

                int pos = viewPointProcess.getParameter(parameter).findValue(new ProcRef(process));
                if (pos != -1)
                {
                    viewPointProcess.getParameter(parameter).removeValue(pos);
                }
            }
        }

        /// <summary>
        /// Removes the provided pipeline, if it exists, from the ViewPoint process.
        /// </summary>
        /// <param name="viewPointProcess">The ViewPoint process.</param>
        /// <param name="procRef">The pipeline to remove.</param>
        public static void RemovePipeline(
            IProcess_SPtr viewPointProcess,
            ProcRef procRef)
        {
            lock (m_lockObject)
            {
                int parameter = 0;

                if (IsOutputAFeatureCollection(procRef))
                {
                    parameter = 1;
                }

                int pos = viewPointProcess.getParameter(parameter).findValue(procRef);
                if (pos != -1)
                {
                    viewPointProcess.getParameter(parameter).removeValue(pos);
                }
            }
        }

        /// <summary>
        /// Determines whether the provided ViewPoint process contains the 
        /// provided pipeline.
        /// </summary>
        /// <param name="viewPointProcess">The ViewPoint process.</param>
        /// <param name="procRef">The pipeline to look for.</param>
        /// <returns>
        /// 	<c>true</c> if the specified ViewPoint process contains 
        /// 	the pipeline as an input; otherwise, <c>false</c>.
        /// </returns>
        public static bool ContainsPipeline(
            IProcess_SPtr viewPointProcess,
            ProcRef procRef)
        {
            lock (m_lockObject)
            {
                if (viewPointProcess.isNull()) return false;
                return PipeUtils.findAsParameter(viewPointProcess, procRef);
            }
        }

        /// <summary>
        /// Determines whether the specified ViewPoint process has any input pipelines.
        /// </summary>
        /// <param name="viewPointProcess">The ViewPoint process.</param>
        /// <returns>
        /// 	<c>true</c> if the specified ViewPoint process has input pipelines; otherwise, <c>false</c>.
        /// </returns>
        public static bool HasPipelines(IProcess_SPtr viewPointProcess)
        {
            lock (m_lockObject)
            {
                Vector_Parameter parameters = viewPointProcess.getParameters();

                foreach (Parameter_SPtr parameter in parameters)
                {
                    if (parameter.getValueCount() > 0)
                    {
                        return true;
                    }
                }

                return false;
            }
        }

        /// <summary>
        /// Gets all of the ViewPointProcess' input pipelines.
        /// </summary>
        /// <param name="viewPointProcess">The ViewPointProcess.</param>
        /// <returns>A vector of pipelines.</returns>
        public static Vector_IProcess GetAllPipelines(
            IProcess_SPtr viewPointProcess)
        {
            lock (m_lockObject)
            {
                IUnknown_SPtr unknown =
                    pyxlib.DynamicPointerCast_PYXCOM_IUnknown(
                    viewPointProcess);
                IViewPoint_SPtr viewPoint =
                    pyxlib.DynamicPointerCast_IViewPoint(unknown);

                return viewPoint.getAllPipelines();
            }
        }

        /// <summary>
        /// Calculates the number of input pipelines.
        /// </summary>
        /// <param name="viewPointProcess">The ViewPoint process.</param>
        /// <returns>The number of input pipelines.</returns>
        public static int NumberOfPipelines(IProcess_SPtr viewPointProcess)
        {
            lock (m_lockObject)
            {
                Vector_Parameter parameters = viewPointProcess.getParameters();
                int numberOfPipelines = 0;

                foreach (Parameter_SPtr parameter in parameters)
                {
                    numberOfPipelines += parameter.getValueCount();
                }

                return numberOfPipelines;
            }
        }

        /// <summary>
        /// Determines whether a pipeline is "complete"; for now this means if 
        /// a pipeline outputs either a feature collection/coverage/camera 
        /// view.
        /// </summary>
        /// <param name="procRef">The pipeline to check.</param>
        /// <returns>
        /// 	<c>true</c> if the pipeline is complete; otherwise, 
        /// 	<c>false</c>.
        /// </returns>
        public static bool IsPipelineComplete(ProcRef procRef)
        {
            Pipeline pipeline = PipelineRepository.Instance.GetByProcRef(procRef);

            return (pipeline != null) && IsPipelineComplete(pipeline);            
        }

        /// <summary>
        /// Determines whether a pipeline is "complete"; for now this means if 
        /// a pipeline outputs either a feature collection/coverage/camera 
        /// view.
        /// </summary>
        /// <param name="pipeline">The pipeline to check.</param>
        /// <returns>
        /// 	<c>true</c> if the pipeline is complete; otherwise, 
        /// 	<c>false</c>.
        /// </returns>
        public static bool IsPipelineComplete(Pipeline pipeline)
        {
            return
                pipeline.ProvidesOutputType(ICoverage.iid) ||
                pipeline.ProvidesOutputType(IFeatureCollection.iid) ||
                pipeline.ProvidesOutputType(IRecordCollection.iid) ||
                pipeline.ProvidesOutputType(ICameraView.iid);
        }

        /// <summary>
        /// Indicates whether a pipeline outputs a feature collection.
        /// </summary>
        /// <param name="procRef">
        /// The pipeline ProcRef.
        /// </param>
        /// <returns>
        /// true, if the pipeline outputs a feature collection.
        /// </returns>
        public static bool IsOutputAFeatureCollection(ProcRef procRef)
        {
            Pipeline pipeline = PipelineRepository.Instance.GetByProcRef(procRef);

            if (pipeline != null)
            {
                return IsOutputAFeatureCollection(pipeline);
            }
            else
            {
                IProcess_SPtr process = PipeManager.getProcess(procRef);

                return (process.isNotNull() && IsOutputAFeatureCollection(process));
            }
        }

        /// <summary>
        /// Indicates whether a pipeline outputs a feature collection.
        /// </summary>
        /// <param name="process">
        /// The pipeline ProcRef.
        /// </param>
        /// <returns>
        /// true, if the pipeline outputs a feature collection.
        /// </returns>
        public static bool IsOutputAFeatureCollection(IProcess_SPtr process)
        {
            return process.ProvidesOutputType(IFeatureCollection.iid) && !process.ProvidesOutputType(ICoverage.iid);
        }

        /// <summary>
        /// Indicates whether a pipeline outputs a feature collection.
        /// </summary>
        /// <param name="pipeline">
        /// The pipeline.
        /// </param>
        /// <returns>
        /// true, if the pipeline outputs a feature collection.
        /// </returns>
        public static bool IsOutputAFeatureCollection(Pipeline pipeline)
        {
            return (    pipeline.ProvidesOutputType(IFeatureCollection.iid) &&
                     ! (pipeline.ProvidesOutputType(ICoverage.iid)));
        }

        /// <summary>
        /// Determines whether the provided pipeline is a coverage that can be visualized.
        /// </summary>
        /// <param name="procRef">The pipeline.</param>
        /// <returns>
        /// 	<c>true</c> if the the pipeline outputs a visualizable coverage; otherwise, <c>false</c>.
        /// </returns>
        private static bool IsOutputAVisualizableCoverage(ProcRef procRef)
        {
            IProcess_SPtr pipeline = PipeManager.getProcess(procRef);

            return IsOutputAVisualizableCoverage(pipeline);
        }

        private static bool IsOutputAVisualizableCoverage(IProcess_SPtr pipeline)
        {
            if (pipeline == null || pipeline.get() == null)
            {
                return false;
            }

            IUnknown_SPtr unknown = pyxlib.DynamicPointerCast_PYXCOM_IUnknown(pipeline);
            if (unknown == null || unknown.get() == null)
            {
                return false;
            }

            ICoverage_SPtr coverage = pyxlib.DynamicPointerCast_ICoverage(unknown);
            if (coverage == null || coverage.get() == null)
            {
                return false;
            }

#if Airborne_Imaging_Demo

            if (coverage.getCoverageDefinition().getFieldCount() <= 0)
            {
                return false;
            }

#else

            if (coverage.getCoverageDefinition().getFieldCount() != 1)
            {
                return false;
            }

#endif


            PYXFieldDefinition definition = 
                coverage.getCoverageDefinition().getFieldDefinition(0);

            if(definition.getContext() == PYXFieldDefinition.eContextType.knContextRGB)
            {
                if ((definition.getType() == PYXValue.eType.knUInt8) && (definition.getCount() == 3 || definition.getCount() == 4))
                {
                    return true;
                }

                if ((definition.getType() != PYXValue.eType.knUInt8) || 
                    (definition.getCount() != 3))
	            {
                    return false;
	            }
            }

            if (definition.getCount() > 4)
                return false;

            /*
             * [chowell] - This is really not that appropriate.  For the time 
             * being to allow testing to get some data out the door we know 
             * that this data is an elevation. We'll need to further be 
             * smarter about how we handle context 'none' in the future.  
             * For example if context is none and by looking at the rest 
             * of the field definition we can see it's RGB should we throw it 
             * away or should we try a best guess? This would hold true for 
             * elevation, greyscale etc. 
	        */            
	        else if((definition.getContext() == PYXFieldDefinition.eContextType.knContextElevation) || 
		        (definition.getContext() == PYXFieldDefinition.eContextType.knContextNone) ||
		        (definition.getContext() == PYXFieldDefinition.eContextType.knContextGreyScale))
	        {
		        if (!definition.isNumeric())
		        {
			        return false;
		        }
	        }
	        else
	        {
		        return false;
	        }

            return true;
        }
        
        /// <summary>
        /// Indicates whether a pipeline outputs a given type.
        /// </summary>
        /// <param name="procRef">
        /// The pipeline ProcRef.
        /// </param>
        /// <param name="outputType">
        /// The type of output type to check.
        /// </param>
        /// <returns>
        /// true, if the pipeline outputs a coverage.
        /// </returns>
        public static bool ProvidesOutputType(ProcRef procRef, GUID outputType)
        {
            Pipeline pipeline = PipelineRepository.Instance.GetByProcRef(procRef);
            if (pipeline != null)
            {
                return (pipeline.ProvidesOutputType(outputType));
            }
            return false;
        }

        /// <summary>
        /// Determines whether the provided pipeline can be visualized.
        /// </summary>
        /// <param name="pipeline">The pipeline.</param>
        /// <returns>
        /// 	<c>true</c> if the provided pipeline can be visualized; otherwise, <c>false</c>.
        /// </returns>
        public static bool CanVisualize(IProcess_SPtr pipeline)
        {
            if(pipeline == null || pipeline.get() == null)
            {
                return false;
            }

            if (!pipeline.ProvidesOutputType(ICoverage.iid) && pipeline.ProvidesOutputType(IFeatureCollection.iid))
            {
                return true;
            }

            return IsOutputAVisualizableCoverage(pipeline);
        }        
    }

    #region Unit Tests

    namespace Test
    {
        using NUnit.Framework;

        /// <summary>
        /// Unit tests for the ViewPointProcessUtility.
        /// </summary>
        [TestFixture]
        public class ViewPointProcessUtilityTester
        {
            private static string s_viewPointProcessGuid = 
                "{85ECCFCB-1D9B-4df9-807F-391D03FCB1FB}";

            private static string s_constCoverageGuid =
                "{8517369E-B91F-46be-BC8A-82E3F414D6AA}";

            /// <summary>
            /// Tests the ViewPointProcessUtility.
            /// </summary>
            [Test]
            public void Test()
            {
                
                IProcess_SPtr viewPointProcess = PYXCOMFactory.CreateProcess(s_viewPointProcessGuid);
                
                // test ViewPointProcess with no inputs
                {
                    Assert.IsFalse(
                        ViewPointProcessUtility.HasPipelines(viewPointProcess),
                        "The ViewPointProcess should have no input pipelines!");

                    Assert.AreEqual(0, 
                        ViewPointProcessUtility.NumberOfPipelines(viewPointProcess));

                    Assert.AreEqual(0,
                        ViewPointProcessUtility.GetAllPipelines(
                        viewPointProcess).Count);
                }

                
                IProcess_SPtr constCoverage = PYXCOMFactory.CreateProcess(s_constCoverageGuid);
                ProcRef procRef = new ProcRef(constCoverage);
                PipeManager.import(constCoverage);

                // test that an input pipeline is added properly to a ViewPointProcess
                {
                    ViewPointProcessUtility.AddPipeline(
                        viewPointProcess, procRef);

                    Assert.IsTrue(
                        ViewPointProcessUtility.HasPipelines(viewPointProcess),
                        "The ViewPointProcess should have 1 input pipeline!");

                    Assert.AreEqual(1, 
                        ViewPointProcessUtility.NumberOfPipelines(viewPointProcess));

                    Assert.AreEqual(1,
                        ViewPointProcessUtility.GetAllPipelines(
                        viewPointProcess).Count);

                    Assert.IsTrue(
                        ViewPointProcessUtility.ContainsPipeline(viewPointProcess, 
                        procRef), 
                        string.Format(
                        "The ViewPointProcess should have '{0}' as an input!", 
                        pyxlib.procRefToStr(procRef)));
                }

                // test that the same pipeline is not added twice
                {
                    ViewPointProcessUtility.AddPipeline(
                        viewPointProcess, procRef);

                    Assert.IsTrue(
                        ViewPointProcessUtility.HasPipelines(viewPointProcess),
                        "The ViewPointProcess should have 1 input pipeline!");

                    Assert.AreEqual(1,
                        ViewPointProcessUtility.NumberOfPipelines(viewPointProcess));

                    Assert.AreEqual(1,
                        ViewPointProcessUtility.GetAllPipelines(
                        viewPointProcess).Count);

                    Assert.IsTrue(
                        ViewPointProcessUtility.ContainsPipeline(viewPointProcess,
                        procRef),
                        string.Format(
                        "The ViewPointProcess should have '{0}' as an input!",
                        pyxlib.procRefToStr(procRef)));
                }

                // test that a pipeline is removed properly from the ViewPointProcess
                {
                    ViewPointProcessUtility.RemovePipeline(
                        viewPointProcess, procRef);

                    Assert.IsFalse(
                        ViewPointProcessUtility.HasPipelines(viewPointProcess),
                        "The ViewPointProcess should have no input pipelines!");

                    Assert.AreEqual(0,
                        ViewPointProcessUtility.NumberOfPipelines(viewPointProcess));

                    Assert.AreEqual(0,
                        ViewPointProcessUtility.GetAllPipelines(
                        viewPointProcess).Count);
                }
            }
        }
    }

    #endregion Unit Tests
}
