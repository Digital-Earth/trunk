/******************************************************************************
IPipelineRepository.cs

begin		: September 29, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Services.PipelineLibrary.Domain
{
    /// <summary>
    /// An interface to be used by all clients to perform all CRUD (create, 
    /// read, update, delete) operations on the PipelineLibrary database.
    /// </summary>
    public interface IPipelineRepository
    {
        /// <summary>
        /// Initializes NHibernate with the specified database configuration 
        /// and begins a new database transaction.
        /// </summary>
        /// <param name="databaseConfiguration">The configuration.</param>
        void Initialize(Dictionary<string, string> databaseConfiguration);

        /// <summary>
        /// Commits pending transaction and starts a new one.
        /// </summary>
        void CheckPoint();

        /// <summary>
        /// Uninitializes, committing any open transaction.
        /// </summary>
        void Uninitialize();

        /// <summary>
        /// Adds the specified process to the database.  
        /// If the provided pipeline's identity is not null, 
        /// and the identity exists in the database, 
        /// the pipeline is associated to that identity and added to the 
        /// database.  If the identity does not exist in the database, the 
        /// identity and then the pipeline is added to the database.  
        /// If the pipeline itself already exists in the database, Update(pipeline) is called.
        /// </summary>
        /// <param name="process">The process to add.</param>
        void Add(IProcess_SPtr process);

        /// <summary>
        /// Gets a list of pipelines from the database that output the type provided.
        /// </summary>
        /// <param name="outputType">The output type.</param>
        /// <returns>
        /// A list of pipelines (an empty list is returned if there are no
        /// matching pipelines).
        /// </returns>
        IList<Pipeline> GetByOutputType(Guid outputType);

        /// <summary>
        /// Gets a pipeline from the database by its ProcRef.
        /// </summary>
        /// <param name="procRef">The ProcRef.</param>
        /// <returns>A pipeline, if it's found in the database, otherwise, null.</returns>
        Pipeline GetByProcRef(ProcRef procRef);

        /// <summary>
        /// Gets a pipeline from the database by its Guid and version.
        /// </summary>
        /// <param name="guid">The Guid.</param>
        /// <param name="version">The version.</param>
        /// <returns>A pipeline, if it's found in the database, otherwise, null.</returns>
        Pipeline GetByProcRef(Guid guid, int version);

        /// <summary>
        /// Gets all pipelines from the database that have the provided identity.
        /// </summary>
        /// <param name="identity">The identity.</param>
        /// <returns>A list of matching pipelines (an empty list is returned 
        /// if there are no matches.)</returns>
        IList<Domain.Pipeline> GetByIdentity(string identity);

        /// <summary>
        /// Gets all the pipelines in the database.
        /// </summary>
        /// <returns></returns>
        IList<Pipeline> GetAllPipelines();

        /// <summary>
        /// Gets all published pipelines in the database.
        /// </summary>
        /// <returns>A list of pipelines (an empty list is returned if there are no 
        /// published pipelines).</returns>
        IList<Pipeline> GetAllPublishedPipelines();

        /// <summary>
        /// Gets all pipelines that are parents and not children of any other pipelines.
        /// </summary>
        /// <returns>A list of parent pipelines (an empty list is returned if there are no 
        /// parent pipelines).</returns>
        IList<Pipeline> GetAllParentPipelines();

        /// <summary>
        /// Gets all pipelines that are not hidden and not temporary.
        /// </summary>
        /// <returns>A list of displayable pipelines (an empty list is returned if there are no 
        /// displayable pipelines).</returns>
        IList<Pipeline> GetAllDisplayablePipelines();

        /// <summary>
        /// Gets all pipelines that are not hidden and not temporary, applying 
        /// a client-provided WHERE clause (for filtering by name and 
        /// description) to the SQL query.
        /// </summary>
        /// <param name="whereClause">The WHERE clause to apply to the SQL query.</param>
        /// <returns>A list of displayable pipelines (an empty list is returned if there are no 
        /// displayable pipelines).</returns>
        IList<Pipeline> GetAllDisplayablePipelines(string whereClause);

        /// <summary>
        /// Gets all pipelines that are not hidden and not temporary, whose identities are not stable.
        /// </summary>
        /// <returns>A list of displayable pipelines with unstable identity (an empty, 
        /// non-null list is returned if none exist).</returns>
        IList<Pipeline> GetAllPipelinesWithUnstableIdentity();

        /// <summary>
        /// Determines whether a pipeline exists in the database.
        /// </summary>
        /// <param name="procRef">The ProcRef of the pipeline to look for in the database.</param>
        /// <returns>true, if the pipeline exists; otherwise, false.</returns>
        bool Exists(ProcRef procRef);

        /// <summary>
        /// Determines whether a pipeline exists in the database.
        /// </summary>
        /// <param name="process">The IProcess_SPtr to look for in the database.</param>
        /// <returns>true, if the pipeline exists; otherwise, false.</returns>
        bool Exists(IProcess_SPtr process);

        /// <summary>
        /// Determines whether there are any pipelines that have the provided identity.
        /// </summary>
        /// <param name="identity">The identity.</param>
        /// <returns>true, if any pipeline(s) are found; otherwise, false.</returns>
        bool Exists(string identity);

        #region Helpful Methods for Clients

        /// <summary>
        /// Gets the pipeline's geometry, if currently available.
        /// </summary>
        /// <param name="procRef">The ProcRef.</param>
        /// <returns>Returns a valid PYXGeometry_SPtr or null.</returns>
        PYXGeometry_SPtr TryGetGeometry(ProcRef procRef);

        /// <summary>
        /// Gets the pipeline's geometry.
        /// </summary>
        /// <param name="procRef">The ProcRef.</param>
        /// <returns>Returns a valid PYXGeometry_SPtr or null.</returns>
        PYXGeometry_SPtr GetGeometry(ProcRef procRef);

        // TODO[kabraman]: This method probably doesn't belong in this interface.
        /// <summary>
        /// Persists the pipeline's geometry to disk, overwriting the existing cache.
        /// </summary>
        /// <param name="procRef">The pipeline whose geometry to persist.</param>
        void PersistGeometry(ProcRef procRef);

        /// <summary>
        /// Sets the published state of a pipeline to the provided value.
        /// </summary>
        /// <param name="procRef">The ProcRef.</param>
        /// <param name="value">The published state</param>
        void SetIsPublished(ProcRef procRef, bool value);

        /// <summary>
        /// Sets the published state of a pipeline to the provided value.
        /// </summary>
        /// <param name="process">The process.</param>
        /// <param name="value">The published state.</param>
        void SetIsPublished(IProcess_SPtr process, bool value);

        /// <summary>
        /// Sets the hidden state of a pipeline to the provided value.
        /// </summary>
        /// <param name="procRef">The ProcRef.</param>
        /// <param name="value">The hidden state.</param>
        void SetIsHidden(ProcRef procRef, bool value);

        /// <summary>
        /// Sets the hidden state of a pipeline to the provided value.
        /// </summary>
        /// <param name="process">The process.</param>
        /// <param name="value">The hidden state.</param>
        void SetIsHidden(IProcess_SPtr process, bool value);

        /// <summary>
        /// Sets the temporary state of a pipeline to the provided value.
        /// </summary>
        /// <param name="procRef">The ProcRef.</param>
        /// <param name="value">The temporary state.</param>
        void SetIsTemporary(ProcRef procRef, bool value);

        /// <summary>
        /// Sets the temporary state of a pipeline to the provided value.
        /// </summary>
        /// <param name="process">The process.</param>
        /// <param name="value">The temporary state.</param>
        void SetIsTemporary(IProcess_SPtr process, bool value);

        /// <summary>
        /// Sets the temporary state of a pipeline to the provided value.
        /// </summary>
        /// <param name="process">The processes.</param>
        /// <param name="value">The temporary state.</param>
        void SetIsTemporary(Vector_IProcess processes, bool value);

        /// <summary>
        /// Try to remove a pipeline from the library.
        /// It returns true only if the pipeline is not used by other pipelines 
        /// and therefore is safe to be removed.
        /// </summary>
        /// <param name="procRef">The procref of the pipeline to be removed.</param>
        bool TryRemovePipeline(ProcRef procRef);

        #endregion Helpful Methods for Clients

        #region Events

        /// <summary>
        /// Raised when a pipeline is added to the database.
        /// </summary>
        event EventHandler<PipelineEventArgs> PipelineAdded;

        /// <summary>
        /// Raised when a pipeline is removed from the database.
        /// </summary>
        event EventHandler<PipelineEventArgs> PipelineRemoved;

        /// <summary>
        /// Raised when a pipeline is marked as hidden in the database.
        /// </summary>
        event EventHandler<PipelineEventArgs> PipelineHidden;

        /// <summary>
        /// Raised when a pipeline is marked as NOT hidden in the database.
        /// </summary>
        event EventHandler<PipelineEventArgs> PipelineUnhidden;

        /// <summary>
        /// Raised when a pipeline's geometry is sucessfully resolved.
        /// </summary>
        event EventHandler<PipelineEventArgs> PipelineGeometryResolved;

        /// <summary>
        /// Rasied when there is a fatal error in PipelineRepository that is un recoverable 
        /// </summary>
        event EventHandler<UnhandledExceptionEventArgs> OnFatalError;

        #endregion Events

        #region PipelineStatus Methods


        void SetIsImported(ProcRef procRef, bool isImported);

        void SetIsDownloaded(ProcRef procRef, bool isDownloaded);

        void SetIsProcessed(ProcRef procRef, bool isProcessed);

        void SetProcessedResolution(ProcRef procRef, int resolution);

        IList<string> GetNotImportedPipelines();

        IList<ProcRef> GetNotDownloadedPipelines();

        IList<ProcRef> GetNotPublishedPipelines();

        IList<ProcRef> GetNotProcessedPipelines();

        #endregion PipelineStatus Methods
    }
}
