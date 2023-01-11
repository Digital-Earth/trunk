namespace GeoStreamService.Jobs
{
    internal interface IJobCancellationHint
    {
        bool ShouldCancel(Job job);
    }

    internal class ProcRefJobCancellationHint : IJobCancellationHint
    {
        private ProcRef m_procRef;

        public ProcRefJobCancellationHint(ProcRef procRef)
        {
            m_procRef = procRef;
        }

        public bool ShouldCancel(Job job)
        {
            var processRelatedjob = job as JobOnProcess;
            if (processRelatedjob != null)
            {
                if (processRelatedjob.ProcRef == m_procRef)
                {
                    return true;
                }
            }
            return false;
        }
    }
}