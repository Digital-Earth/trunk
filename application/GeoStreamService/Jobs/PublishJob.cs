/******************************************************************************
PublishJob.cs

begin		: February 9, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Services.PipelineLibrary.Repositories;
using System;

namespace GeoStreamService.Jobs
{
    /// <summary>
    /// Responsible for publishing the pipeline so that its tiles can be served out.
    /// </summary>
    internal class PublishJob : JobOnProcess
    {
        public string PipelineName;

        public override bool Equals(object obj)
        {
            if (obj is PublishJob &&
                ((PublishJob)(obj)).PipelineName == PipelineName &&
                ((PublishJob)(obj)).ProcRef == ProcRef)
            {
                return true;
            }
            return base.Equals(obj);
        }

        public static bool operator !=(PublishJob a, PublishJob b)
        {
            return !(a == b);
        }

        public static bool operator ==(PublishJob a, PublishJob b)
        {
            if (object.ReferenceEquals(a, null))
            {
                return object.ReferenceEquals(b, null);
            }
            return a.Equals(b);
        }

        public override int GetHashCode()
        {
            if (ProcRef != null)
            {
                return PipelineName.GetHashCode() * ProcRef.GetHashCode();
            }
            else
            {
                return 0;
            }
        }

        public PublishJob(IProcess_SPtr process, string name)
            : base()
        {
            Process = process;
            ProcRef = new ProcRef(process);
            PipelineName = name;

            Status = new ObservableOperationStatus();
            Status.Operation.OperationType = Pyxis.Contract.Operations.OperationType.Publish;
            Status.Operation.Parameters.Add("ProcRef", pyxlib.procRefToStr(ProcRef));
            Status.Description = String.Format("Publish pipeline '{0}={1}'", PipelineName, ProcRef);
        }

        protected override void DoExecute()
        {
            try
            {
                if (PyxNet.Pyxis.PublisherSingleton.Publisher.Publish(ProcRef))
                {
                    GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Publish, "'{0}={1}' is a newly published pipeline.",
                        PipelineName, pyxlib.procRefToStr(ProcRef));
                }
                else
                {
                    GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Publish,
                        "'{0}={1}' was already published by this node (ignoring republish).",
                        PipelineName, pyxlib.procRefToStr(ProcRef));
                }
            }
            catch (Exception ex)
            {
                throw new Exception(
                    String.Format(
                        "\nAn unexpected error occurred when attempting to publish {0}={1}; " +
                        "the exception info is printed below:\n{2}",
                        PipelineName, pyxlib.procRefToStr(ProcRef), ex.ToString()), ex);
            }
        }
    }
}