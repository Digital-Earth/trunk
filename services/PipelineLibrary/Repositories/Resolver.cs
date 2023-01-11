/******************************************************************************
Resolver.cs

begin		: October 8, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Services.PipelineLibrary.Repositories
{
    // TODO[kabiraman]: No need for LibraryProcessResolver.  We should add 
    // C# directors support to ProcessResolver and inherit directly from it.
    /// <summary>
    /// Belongs to the chain of ProcessResolvers that includes the Memory 
    /// and LocalDisk Resolvers.
    /// </summary>
    public class Resolver : LibraryProcessResolver
    {
        public override IProcess_SPtr resolve(ProcRef procRef)
        {
            IProcess_SPtr process = null;

            try
            {
                Domain.Pipeline pipeline = PipelineRepository.Instance.GetByProcRef(
                    procRef);

                if (pipeline != null && !string.IsNullOrEmpty(pipeline.Definition))
                {
                    process = PipeManager.readProcessFromString(pipeline.Definition);                        
                }
            }
            catch (Exception ex)
            {
                Trace.error(string.Format(
                    "Error during LibraryProcessResolver resolve: {0}", 
                    ex.ToString()));
            }

            if (process == null)
            {
                //if process is null, SWIG will raise an exception - part of the contract in resolve it just it return an object.
                process = new IProcess_SPtr();
            }

            return process;
        }

        public override IProcess_SPtr notifyResolve(IProcess_SPtr process)
        {
            try
            {
                Domain.Pipeline pipeline = 
                    PipelineRepository.Instance.GetByProcRef(new ProcRef(process));

                if (pipeline == null)
                {
                    PipelineRepository.Instance.Add(process);
                }
            }
            catch (Exception ex)
            {
                Trace.error(string.Format(
                    "Error during LibraryProcessResolver notifyResolve: {0}",
                    ex.ToString()));
            }

            return process;
        }


        static protected List<Resolver> s_unmanagedReferences =
            new List<Resolver>();

        public override int addRef()
        {
            int refCount = base.addRef();

            if (refCount == 2)
            {
                swigCMemOwn = false;
                s_unmanagedReferences.Add(this);
            }

            return refCount;
        }

        public override int release()
        {
            int refCount = base.release();

            if (refCount == 1)
            {
                swigCMemOwn = true;
                s_unmanagedReferences.Remove(this);
            }

            return refCount;            
        }
    }
}
