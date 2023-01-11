/******************************************************************************
ReportStatusJob.cs

begin		: July 9, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using Microsoft.Practices.Unity;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Services.GeoWebStreamService;
using Pyxis.Contract.Services.LicenseService;
using Pyxis.Publishing.Protocol;
using System.Threading.Tasks;

namespace GeoStreamService.Jobs
{
    /// <summary>
    /// Responsible for communicating to license server - reporting the status and processing the LS requests
    /// </summary>
    internal class ReportStatusJob : Job
    {
        [Dependency]
        public IGwssClient m_licenseServer { get; set; }

        /// <summary>
        /// Function that is able to generate the current GwssReport
        /// </summary>
        private Func<IGwssStatus> GenerateGwssReport { get; set; }

        /// <summary>
        /// Gwss we will about to send, it will be created by GenerateGwssReport Function when the job is about to be exectued.
        /// </summary>
        public IGwssStatus GwssReport { get; private set; }

        public ILsStatus Response { get; private set; }

        public ReportStatusJob(Func<IGwssStatus> getReport)
            : base()
        {
            Status = new ObservableOperationStatus();
            Status.Description = "Report GWSS status";
            GenerateGwssReport = getReport;
        }

        public delegate void HandleResponse(ILsStatus Response);
        public event HandleResponse ResponseReceived;

        protected override void DoExecute()
        {
            Response = null;
            GwssReport = GenerateGwssReport();
            var task = Task.Factory.StartNew(() => { Response = m_licenseServer.UpdateStatus(GwssReport); });
            task.ContinueWith(t =>
                {
                    if (Response == null)
                    {
                        Status.StatusCode = OperationStatusCode.Failed;
                    }
                    else
                    {
                        Status.StatusCode = OperationStatusCode.Completed;
                        ResponseReceived(Response);
                    }
                });
            TimeSpan waitTime = TimeSpan.FromSeconds(5);
            if (!task.Wait(waitTime))
            {
                GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, "No response was received from LS within " + waitTime.TotalSeconds + " seconds");
            }
        }
    }
}