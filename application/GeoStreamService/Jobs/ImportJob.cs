/******************************************************************************
ImportJob.cs

begin		: February 9, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Contract.Operations;
using Pyxis.Publishing.Protocol;
using Pyxis.Services.PipelineLibrary.Repositories;
using System;

namespace GeoStreamService.Jobs
{
    /// <summary>
    /// Responsible for importing a published pipeline to the database.
    /// </summary>
    internal class ImportJob : JobOnProcess
    {

        private IPipelineClient m_pipelineClient;

        private string m_pipelineDefinition;

        public ImportJob(ProcRef procRef, IPipelineClient pipelineClient)
            : base()
        {
            ProcRef = procRef;
            m_pipelineClient = pipelineClient;

            Status = new ObservableOperationStatus();
            Status.Operation.OperationType = OperationType.Import;
            Status.Operation.Parameters.Add("ProcRef", procRef.ToString());
            Status.Description = "Import Pipeline :" + procRef.ToString();
        }

        public static bool operator !=(ImportJob a, ImportJob b)
        {
            return !(a == b);
        }

        public static bool operator ==(ImportJob a, ImportJob b)
        {
            if (object.ReferenceEquals(a, null))
            {
                return object.ReferenceEquals(b, null);
            }
            return a.Equals(b);
        }

        public override bool Equals(object obj)
        {
            if (obj != null && obj is ImportJob && ProcRef == ((ImportJob)obj).ProcRef)
            {
                return true;
            }
            return base.Equals(obj);
        }

        public override int GetHashCode()
        {
            if (m_pipelineDefinition != null)
            {
                return m_pipelineDefinition.GetHashCode();
            }
            else
            {
                return 0;
            }
        }

        protected override void DoExecute()
        {
            var metadata = m_pipelineClient.GetPipelineMetaData(ProcRef.ToString());
            m_pipelineDefinition = metadata.Definition;

            if (String.IsNullOrEmpty(m_pipelineDefinition))
            {
                throw new Exception("Failed to resolve definition for ProcRef: " + ProcRef.ToString());
            }

            string pipelineDefinition = m_pipelineDefinition;

            Vector_IProcess processes = PipeManager.importStr(pipelineDefinition);

            PipelineRepository.Instance.SetIsTemporary(processes, false);

            PipeUtils.pruneNonRoots(processes);

            System.Diagnostics.Debug.Assert(
                processes.Count == 1,
                String.Format("OnPublishMessage() should only ever be raised for 1 pipeline at a time.  " +
                "Instead {0} pipelines were detected.",
                processes.Count));

            var importedProcRef = new ProcRef(processes[0]);
            if (ProcRef != importedProcRef)
            {
                throw new Exception("Imported procref is different from the definition head! requested procref:" + ProcRef + " definition head:" + importedProcRef);
            }

            Status.Description = "Import Pipeline ProcRef:" + pyxlib.procRefToStr(ProcRef) + ", name: " + processes[0].getProcName();

            PipelineRepository.Instance.SetIsImported(ProcRef, true);
            PipelineRepository.Instance.CheckPoint();

            //The swig Vector_IProcess do not increase the reference so here we make a copy of the process.
            IProcess_SPtr[] array = new IProcess_SPtr[1];
            processes.CopyTo(0, array, 0, 1);
            Process = array[0];
        }
    }
}