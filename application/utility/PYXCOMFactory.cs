using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using Pyxis.Utilities;

namespace ApplicationUtility
{
    public class PYXCOMProcessCreateInfo
    {
        public GUID ProcessGuid { get; set; }
        public Dictionary<string, string> Attributes { get; set; }
        public Dictionary<int, List<IProcess_SPtr>> Inputs { get; set; }

        public string ProcessName { get; set; }
        public string ProcessDescription { get; set; }

        public PYXCOMProcessCreateInfo(GUID guid)
        {
            ProcessGuid = guid;
            Attributes = new Dictionary<string, string>();
            Inputs = new Dictionary<int, List<IProcess_SPtr>>();
        }

        public PYXCOMProcessCreateInfo(string guid) : this(pyxlib.strToGuid(guid))
        {  
        }

        public PYXCOMProcessCreateInfo AddAttribute(string name,string value)
        {
            Attributes[name] = value;
            return this;
        }

        public PYXCOMProcessCreateInfo BorrowNameAndDescription(IProcess_SPtr process)
        {
            SetName(process.getProcName());
            SetDescription(process.getProcDescription());
            return this;
        }

        public PYXCOMProcessCreateInfo BorrowInputs(IProcess_SPtr process)
        {
            var parameterCount = process.getParameterCount();
            for (int i = 0; i < parameterCount; i++)
            {
                var parameter = process.getParameter(i);

                if (parameter.getValueCount() > 0)
                {
                    foreach (var value in parameter.getValues())
                    {
                        AddInput(i, value);
                    }
                }
            }
            return this;
        }

        public PYXCOMProcessCreateInfo AddInput(int paramIndex, IProcess_SPtr value)
        {
            if (!Inputs.ContainsKey(paramIndex))
                Inputs[paramIndex] = new List<IProcess_SPtr>();
            Inputs[paramIndex].Add(value);
            return this;
        }

        public PYXCOMProcessCreateInfo SetName(string name)
        {
            ProcessName = name;
            return this;
        }

        public PYXCOMProcessCreateInfo SetDescription(string desc)
        {
            ProcessDescription = desc;
            return this;
        }
    }

    public class PYXCOMFactory
    {
        public static IUnknown_SPtr CreateInstance(string guid)
        {
            return CreateInstance(pyxlib.strToGuid(guid));
        }

        public static IUnknown_SPtr CreateInstance(GUID guid)
        {
            return pyxlib.PYXCOMhelpCreate(guid);
        }

        public static IProcess_SPtr CreateProcess(string guid)
        {
            return CreateProcess(pyxlib.strToGuid(guid));
        }

        public static IProcess_SPtr CreateProcess(GUID guid)
        {
            return pyxlib.QueryInterface_IProcess(CreateInstance(guid));
        }

        public static IProcess_SPtr CreateProcess(PYXCOMProcessCreateInfo info)
        {
            IProcess_SPtr process = CreateProcess(info.ProcessGuid);

            if (info.Attributes.Count > 0)
            {
                var attributeMap = new ManagedAttributeMap();
                foreach (var attr in info.Attributes)
                {
                    attributeMap[attr.Key] = attr.Value;
                }
                process.setAttributes(attributeMap.AttributeMap);
            }

            foreach(var paramter in info.Inputs)
            {
                var processParam = process.getParameter(paramter.Key);
                foreach (var value in paramter.Value)
                {
                    processParam.addValue(value);
                }
            }

            if (!String.IsNullOrEmpty(info.ProcessName))
            {
                process.setProcName(info.ProcessName);
            }

            if (!String.IsNullOrEmpty(info.ProcessDescription))
            {
                process.setProcDescription(info.ProcessDescription);
            }

            return process;
        }

        public static class WellKnownInterfaces
        {
            public static GUID ISRS = pyxlib.strToGuid("{7BE2766C-3B77-44E1-B15F-76D8F6E54D06}");
            public static GUID IOWSReference = pyxlib.strToGuid("{CFB38CB7-586F-4895-BAA9-D2E1EBA810B3}");
            public static GUID IGeoServicesReference = pyxlib.strToGuid("{BC89116A-FF90-4015-A551-0443F2162435}");
        }

        public static class WellKnownProcesses
        {
            public static GUID ViewPointProcess = pyxlib.strToGuid("{85ECCFCB-1D9B-4DF9-807F-391D03FCB1FB}");
            public static GUID DocumentProcess = pyxlib.strToGuid("{D4550BA5-B5B8-4950-A198-CEA258FB6BC2}");

            public static GUID CoverageCache = pyxlib.strToGuid("{83F35C37-5D0A-41C9-A937-F8C9C1E86850}");
            public static GUID FeaturesSummary = pyxlib.strToGuid("{E6C3802D-E7B3-431C-A41F-FBAB79E1CA2D}");
            
            public static GUID CoverageFirstNotNull = pyxlib.strToGuid("{79E1D5B2-F816-449E-876B-9EAF0B1CE118}");
            public static GUID ConcatFeatures = pyxlib.strToGuid("{BBDCA91A-083E-4A86-B694-6E808A62DC07}");

            public static GUID ClampledSampler = pyxlib.strToGuid("{03A11842-7EE9-400E-9CA1-A4942E99E8FE}");
            public static GUID NearestNeighbourSampler = pyxlib.strToGuid("{D612004E-FC51-449A-B0D6-1860E59F3B0D}");
            public static GUID ClampledSamplerWithNulls = pyxlib.strToGuid("{E414C479-4E9A-423F-AA17-6ABF54967705}");
            public static GUID BilinearSampler = pyxlib.strToGuid("{46C8C829-9CFE-469E-93DF-F9688B83BB09}");
            public static GUID BicubicSampler = pyxlib.strToGuid("{175AB4E6-BDE1-4C30-9C80-BC9A31E45058}");
            public static GUID BicubicSamplerWithNulls = pyxlib.strToGuid("{80B2DD06-1A5F-40EE-A9C7-FEB3B658A80C}");

            public static GUID Url = pyxlib.strToGuid("{BB6C61B0-67E2-4072-8A6C-D24AE68F5B0E}");
            public static GUID Path = pyxlib.strToGuid("{7B50BE20-F6A5-401E-8E76-98C5226DAF0D}");
            public static GUID SRS = pyxlib.strToGuid("{25D72D8D-B26F-428B-9FDF-408C7C73791D}");

            public static GUID ToolBoxProvider = pyxlib.strToGuid("{D7D6DDE7-E439-46B9-A075-0A16FF9DC64F}");
            public static GUID ProcessCollection = pyxlib.strToGuid("{F9C16F11-D897-4CE2-A6EC-70E286F0D194}");
            public static GUID UserCredentialsProvider = pyxlib.strToGuid("{34A7E82B-E78F-48FD-8B19-9B024D5E4E7F}");

            public static GUID StyledCoverage = pyxlib.strToGuid("{43FFAAE3-0A08-45F8-80DA-2E75B31EB96F}");
            public static GUID StyledFeaturesSummary = pyxlib.strToGuid("{4F41A149-7EBF-41CB-B0F9-031D76EF81E0}");
            public static GUID IconStyledFeatureCollection = pyxlib.strToGuid("{705C8518-FDDD-4946-840D-96515B9D29BD}");
            public static GUID LineStyledFeatureCollection = pyxlib.strToGuid("{4C610229-D1EA-4307-9A53-E5BAA9FE8BB5}");
            public static GUID AreaStyledFeatureCollection = pyxlib.strToGuid("{A7B8FC7A-6041-4A15-BD97-7F4A976D91AD}");
            public static GUID BehaviourStyledFeatureCollection = pyxlib.strToGuid("{8A168956-0431-493F-B0E1-8354F65CFC82}");

            public static GUID AggregateFeatureCalculator = pyxlib.strToGuid("{AEEBE10E-8A51-45D7-9BBD-FDB6D584C245}");
            public static GUID AggregateCoverageCalculator = pyxlib.strToGuid("{481612EA-3018-42EE-9B56-32601A3051BE}");
            public static GUID ModifyFeaturePropertiesProcess = pyxlib.strToGuid("{DF99F5EE-4C75-41C2-AC08-E3F5DEB808D1}");
            public static GUID Calculator = pyxlib.strToGuid("{FDA4208F-0042-4B4B-86A8-B4DBEFA43733}");
            public static GUID FeatureCollectionIndex = pyxlib.strToGuid("{AFE6F82A-8E82-41CA-9764-B45DA5264D76}");                       

            public static GUID OGRWFSProcess = pyxlib.strToGuid("{AA47A7D3-6749-4BD4-99B4-9B4B6CF3EF9C}");
            public static GUID GDALWMSProcess = pyxlib.strToGuid("{F5E595F7-B58B-4D23-AC2B-865829306E10}");
            public static GUID GDALWCSProcessV2 = pyxlib.strToGuid("{3C11ACF7-AD4B-4BF5-A7FA-98ADCD454FDC}");

            public static GUID OGRFeatureServerProcess = pyxlib.strToGuid("{88C80CEE-5EBE-4472-8E5F-FF0D6DD0A935}");

            public static GUID GDALBingProcess = pyxlib.strToGuid("{68EDBF04-72BD-4009-B388-834B8A6AE3C5}");

            public static GUID ExcelRecordCollectionProcess = pyxlib.strToGuid("{4F5B6B1C-67C7-40A6-8E98-2FF511D38AAF}");

            public static GUID StyledFeaturesRasterizer = pyxlib.strToGuid("{E82F5DB8-3BF9-48B5-B529-64BB7819DD38}");

            public static GUID GDALFileProcess = pyxlib.strToGuid("{73B180AB-100B-4742-BE18-045F1DEF4B95}");
            public static GUID OGRProcess = pyxlib.strToGuid("{C621458A-9E1D-41eb-B01E-C0569743C0B8}");
        }

        public static class WellKnownProcessInitError
        {
            public const string ProcSpecFailureErrorID = "{97EE0643-7092-46cb-AE2A-DB9C6665D70D}";
            public const string MissingSRSErrorID = "{68F0FC89-2D83-439C-BD4E-72A8A9CCDCED}";
            public const string MissingUserCredentialsErrorID = "{100B9867-D8E2-4A72-81DE-DE8FEB5187FD}";
            public const string MissingGeometryErrorID = "{62878998-D2B8-4F98-BA48-7ECAD2B523F0}";
            public const string MissingWorldFileErrorID = "{ACA50CE2-E822-49D2-AFE1-1AE5BA7966E9}";
        }

        public static ICoordConverter_SPtr CreateCoordConvertorFromWKT(string wkt)
        {
            var factory = pyxlib.QueryInterface_ICoordConverterFromSrsFactory(CreateInstance("{9FFF0792-6B6B-47B6-89BC-739D0F1137C7}"));
            var coordConvertor = factory.createFromWKT(wkt);
            return coordConvertor;
        }

        public static ICoordConverter_SPtr CreateCoordConvertorFromSRS(PYXSpatialReferenceSystem_SPtr srs)
        {
            var factory = pyxlib.QueryInterface_ICoordConverterFromSrsFactory(CreateInstance("{9FFF0792-6B6B-47B6-89BC-739D0F1137C7}"));
            var coordConvertor = factory.createFromSRS(srs);
            return coordConvertor;
        }

        public static PYXSpatialReferenceSystem_SPtr CreateSRSFromCoordConverter(ICoordConverter_SPtr convertor)
        {
            var factory = pyxlib.QueryInterface_ICoordConverterFromSrsFactory(CreateInstance("{9FFF0792-6B6B-47B6-89BC-739D0F1137C7}"));
            var srs = factory.convertToSRS(convertor);
            return srs;
        }

        public static IProcess_SPtr CreateBitmapPipelineFromUrl(string url)
        {
            //URLProcess
            var path = CreateProcess(
                new PYXCOMProcessCreateInfo(UrlProcess.clsid)
                    .AddAttribute("urlString", url)
                );

            //BitmapProcess
            IProcess_SPtr bitmapProcess;

            if (url.StartsWith("data:"))
            {
                bitmapProcess = CreateProcess(
                    new PYXCOMProcessCreateInfo("{5A0BC809-48AF-4152-B22A-E12ED3A059E7}")
                        .AddInput(0, path)
                        .SetName("embedded image")
                        .SetDescription("embedded image")
                    );
            }
            else
            {
                bitmapProcess = CreateProcess(
                    new PYXCOMProcessCreateInfo("{5A0BC809-48AF-4152-B22A-E12ED3A059E7}")
                        .AddInput(0, path)
                        .SetName(url)
                        .SetDescription("Read Bitmaps from url : " + url + ".")
                    );
            }

            return bitmapProcess;
        }

        public static IProcess_SPtr CreateBitmapPipelineFromPath(string filename)
        {
            //PathProcess
            var path = CreateProcess(
                new PYXCOMProcessCreateInfo(PathProcess.clsid)
                    .AddAttribute("uri", filename)
                );

            //BitmapProcess
            var bitmapProcess = CreateProcess(
                new PYXCOMProcessCreateInfo("{5A0BC809-48AF-4152-B22A-E12ED3A059E7}")
                .AddInput(0, path)
                .SetName(Path.GetFileName(filename))
                .SetDescription("Read Bitmaps from file at path: " + filename + ".")
                );

            return bitmapProcess;
        }

        public static IProcess_SPtr GetProcess(ProcRef procRef)
        {
            return PipeManager.getProcess(procRef, true);
        }

        public static IProcess_SPtr GetProcess(ProcRef procRef,bool initialize)
        {
            return PipeManager.getProcess(procRef, initialize);
        }

        public static GetProcessTask GetProcessAsync(ProcRef procRef)
        {
            return GetProcessTask.Create(procRef);
        }


        public static IProcess_SPtr CreateSimpleFeature(string id, PointLocation point, IEnumerable<KeyValuePair<string, PYXValue>> properties = null)
        {
            return CreateSimpleFeature(id, pyxlib.DynamicPointerCast_PYXGeometry(PYXVectorGeometry2.createFromPoint(point.asXYZ(), 24)), properties);
        }

        public static IProcess_SPtr CreateSimpleFeature(string id, PYXGeometry_SPtr geometry, IEnumerable<KeyValuePair<string, PYXValue>> properties = null)
        {
            var process = CreateProcess("{2309AE25-FE1A-482f-9642-D2ADD1520629}");  

            var writeableFeature = pyxlib.QueryInterface_IWritableFeature(process.getOutput());

            writeableFeature.setID(id);
            writeableFeature.setGeometry(geometry);
            
            if (properties != null)
            {
                foreach (var prop in properties)
                {
                    if (prop.Value == null)
                    {
                        throw new Exception("CreateSimpleFeature does not support null values, use nullable<T> instead");
                    }
                    writeableFeature.addField(prop.Key, PYXFieldDefinition.eContextType.knContextNone, prop.Value.getType(), prop.Value.getArraySize(), prop.Value);
                }
            }

            return process;
        }

        public static IProcess_SPtr CreateSimpleFeatureCollection(IEnumerable<IProcess_SPtr> features)
        {
            var featureCollectionProc = CreateProcess("{DEF42C63-F377-4065-8C58-7215FBA45222}");

            var inputParameter = featureCollectionProc.getParameter(0);
            foreach(var feature in features) {
                inputParameter.addValue(feature);
            }

            return featureCollectionProc;
        }
    }

    public class GetProcessTask
    {
        static private readonly Dictionary<string,GetProcessTask> s_activeTasks = new Dictionary<string, GetProcessTask>();
        static private readonly Dictionary<string, GetProcessTask> s_completedTasks = new Dictionary<string, GetProcessTask>();
        static private readonly List<string> s_completedTasksLsu = new List<string>();
        static private readonly object s_activeTasksLock = new object();

        private readonly SynchronizationEvent m_sync = new SynchronizationEvent();
        private IProcess_SPtr m_result;
        private Exception m_error;

        public ProcRef ProcRef { get; private set; }
        public IProcess_SPtr Process
        {
            get
            {
                m_sync.Wait();
                if (m_error != null)
                {
                    throw m_error;
                }
                return m_result;
            }
        }

        public bool Completed
        {
            get
            {
                lock (s_activeTasksLock)
                {
                    return m_result != null;
                }
            }
        }

        public static GetProcessTask Create(ProcRef procRef)
        {
            //add reference
            lock(s_activeTasksLock)
            {
                var key = pyxlib.procRefToStr(procRef);

                if (s_activeTasks.ContainsKey(key))
                {
                    var aTask = s_activeTasks[key];
                    if (aTask.Completed && aTask.Process.isNotNull())
                    {
                        return aTask;
                    }
                }
                if (s_completedTasks.ContainsKey(key))
                {
                    s_completedTasksLsu.Remove(key);
                    var aTask = s_completedTasks[key];

                    if (aTask.Process.isNotNull())
                    {
                        s_completedTasksLsu.Insert(0, key);
                        return aTask;
                    }
                }

                var task = new GetProcessTask(procRef);

                s_activeTasks[key] = task;

                return task;
            }
        }

        private GetProcessTask(ProcRef procRef)
        {
            ProcRef = procRef;
            ThreadPool.QueueUserWorkItem(CreateProcess);
        }

        private void CreateProcess(object state)
        {
            try
            {
                var result = PYXCOMFactory.GetProcess(ProcRef);

                lock (s_activeTasksLock)
                {
                    m_result = result;
                }
            }
            catch (Exception ex)
            {
                m_error = ex;
            }
            m_sync.Pulse();

            //remove reference...
            lock(s_activeTasksLock)
            {
                var key = pyxlib.procRefToStr(ProcRef);
                if (s_activeTasks.ContainsKey(key))
                {
                    s_activeTasks.Remove(key);
                }
                s_completedTasks[key] = this;
                s_completedTasksLsu.Insert(0, key);
                if (s_completedTasksLsu.Count>100)
                {
                    s_completedTasksLsu.RemoveRange(99,s_completedTasksLsu.Count-100);
                }
            }
        }

        public void Wait()
        {
            m_sync.Wait();
        }
    }
}
