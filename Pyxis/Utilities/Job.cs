/******************************************************************************
Job.cs

begin		: January 13, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// The base class for all jobs to be used with <see cref="Pyxis.Utilities.JobList"/>.
    /// </summary>
    public abstract class Job
    {
        /// <summary>
        /// Flag to set when a job is finished.
        /// </summary>
        public Pyxis.Utilities.ObservableObject<bool> Finished =
            new Pyxis.Utilities.ObservableObject<bool>(false);

        /// <summary>
        /// Gets or sets any exception raised during the execution of the 
        /// job.  This exception is thrown to be caught by the creator of 
        /// the job.
        /// </summary>
        /// <value>The exception.</value>
        public Exception Exception { get; set; }

        /// <summary>
        /// Inheritors must define the logic for execute here.
        /// </summary>
        protected virtual void DoExecute()
        {
        }

        /// <summary>
        /// Executes the job, catching any exceptions.
        /// </summary>            
        public virtual void Execute()
        {
            try
            {
                DoExecute();
            }
            catch (Exception ex)
            {
                Exception = ex;
            }
        }
    }
}
