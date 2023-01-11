using System;
using System.Collections.Generic;
using ApplicationUtility;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.IO.DataDiscovery;
using Pyxis.IO.Import;

namespace Pyxis.IO.Sources.Local
{
    internal class LocalDataSetImportService : IDataSetImportService
    {
        public DataSet DataSet { get; set; }
        public IPermit Permit { get; set; }

        public LocalDataSetImportService(DataSet dataSet, IPermit permit)
        {
            DataSet = dataSet;
            Permit = permit;
        }

        /// <summary>
        /// Create a PYXDataSet from a DataSet.
        /// </summary>
        /// <param name="dataSet">The local data set</param>
        /// <returns>A newly created table definition</returns>
        static public PYXDataSet_SPtr ToPyxis(DataSet dataSet)
        {
            if (dataSet != null)
            {
                PYXDataSet_SPtr pyxDataSet = PYXDataSet.create();

                // set url and name
                pyxDataSet.setUri(dataSet.Uri);
                if (dataSet.Metadata != null && dataSet.Metadata.Name.HasContent())
                {
                    pyxDataSet.setName(dataSet.Metadata.Name);
                }

                // set layer
                if (dataSet.Layer != null)
                {
                    pyxDataSet.setLayer(dataSet.Layer);
                }

                if (dataSet.Fields != null)
                {
                    // set fields
                    foreach (var field in dataSet.Fields)
                    {
                        // TODO extract proper type definition when DataSet contains a PipelineSpecification
                        pyxDataSet.getContentDefinition().addFieldDefinition(
                            field,
                            PYXFieldDefinition.eContextType.knContextNone,
                            PYXValue.eType.knNull,
                            1);
                    }
                }

                return pyxDataSet;
            }

            return null;
        }

        public IProcess_SPtr BuildPipeline(Engine engine, IImportSettingProvider settingsProvider)
        {
            var config = CreateBuilderConfig(settingsProvider);

            try
            {
                // check if the data source is supported
                if (PipelineConstructionUtility.IsDataSourceSupported(DataSet.Uri, IPipeBuilder.eCheckOptions.knLenient))
                {
                    // Create a pipeline for importing the data set
                    var pyxDataSet = ToPyxis(DataSet);
                    var result = PipelineConstructionUtility.BuildPipeline(pyxDataSet, config);
                    if (result != null && !result.isNull())
                    {
                        // let the result handler distinguish between the pipelines by the description
                        result.setProcDescription(DataSet.Uri);
                    }

                    return result;
                }
            }
            catch (Exception e)
            {
                Trace.error(String.Format("Failed to create a pipeline for {0}: {1}", DataSet.Uri, e.Message));
            }

            return null;
        }

        private static Dictionary<string, string> CreateBuilderConfig(IImportSettingProvider settingsProvider)
        {
            Dictionary<string, string> config = null;
            if (settingsProvider != null)
            {
                var samplerSettingsTask = settingsProvider.ProvideSetting(typeof(SamplerImportSetting), null);
                if (samplerSettingsTask != null)
                {
                    var samplerSettings = (samplerSettingsTask.Result as SamplerImportSetting);
                    if (samplerSettings != null)
                    {
                        config = new Dictionary<string, string>()
                        {
                            {"default_sampler", samplerSettings.Sampler}
                        };
                    }
                }
            }
            return config;
        }

        public void EnrichGeoSource(Engine engine, GeoSource geoSource)
        {
            if (geoSource.Specification.OutputType == PipelineSpecification.PipelineOutputType.Feature)
            {
                geoSource.Style = engine.CreateDefaultStyle(geoSource);
            }

            return;
        }
    }
}