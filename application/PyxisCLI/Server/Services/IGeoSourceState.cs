using System;
using System.Threading.Tasks;
using Pyxis.Contract.Publishing;
using PyxisCLI.Server.Models;

namespace PyxisCLI.Server.Services
{
    /// <summary>
    /// Base interface to access information about a GeoSource
    /// </summary>
    public interface IGeoSourceState
    {
        /// <summary>
        /// Get the id of the GeoSource
        /// </summary>
        Guid Id { get; }

        /// <summary>
        /// Get a Task that will return the GeoSource object and metadata
        /// </summary>
        /// <returns>Task to get GeoSource object and metadata</returns>
        Task<GeoSource> GetGeoSource();

        /// <summary>
        /// Get a Task that will return the IProcess from the given GeoSource
        /// </summary>
        /// <returns>Task to get IProcess_SPtr</returns>
        Task<IProcess_SPtr> GetProcess();

        /// <summary>
        /// Get a Task that will return the Style for the given geoSource
        /// </summary>
        /// <returns>Task to get default Style</returns>
        Task<Style> GetStyle();

        /// <summary>
        /// Get a Task that will return the Style for the given geoSource based on a reference style
        /// </summary>
        /// <returns>Task to get Style for a GeoSource based on a reference style</returns>
        Task<Style> GetStyle(Style basedOn);

        /// <summary>
        /// Get a Task that will return the Style for the given geoSource based on a field
        /// </summary>
        /// <returns>Task to get Style a geosource based on a field</returns>
        Task<Style> GetStyle(AutoStyleRequest styleRequest);

        /// <summary>
        /// Get a task that will return GeoSourceDataCharacterization object
        /// </summary>
        /// <returns>Task to get GeoSourceDataCharacterization object</returns>
        Task<GeoSourceDataCharacterization> GetCharacterization();

        /// <summary>
        /// Get a task that will return PipelineSpecification object
        /// </summary>
        /// <returns>Task to get PipelineSpecification object</returns>
        Task<PipelineSpecification> GetSpecification();
    }
}
