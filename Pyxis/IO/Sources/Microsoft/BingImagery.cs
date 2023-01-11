using System;
using System.Collections.Generic;
using System.ComponentModel;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Publishing.Permits;

namespace Pyxis.IO.Sources.Microsoft
{
    /// <summary>
    /// A utility for creating a Pyxis.Contract.Publishing.GeoSource for Microsoft Bing imagery.
    /// </summary>
    public class BingImagery
    {
        /// <summary>
        /// Defines the style of the Microsoft Bing imagery.
        /// </summary>
        public enum Style {
            /// <summary>
            /// Aerial imagery.
            /// </summary>
            [Description("Aerial")]
            Aerial,

            /// <summary>
            /// Aerial imagery with labels.
            /// </summary>
            [Description("Aerial With Labels")]
            AerialWithLabels,

            /// <summary>
            /// Roads imagery.
            /// </summary>
            [Description("Roads")]
            Road
        }

        private Engine Engine { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.IO.Microsoft.BingImagery.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine used for creating the Microsoft Bing imagery.</param>
        public BingImagery(Engine engine)
        {
            Engine = engine;
        }

        /// <summary>
        /// Create a Pyxis.Contract.Publishing.GeoSource for Microsoft Bing Imagery of the specified style.
        /// </summary>
        /// <param name="style">The desired Pyxis.IO.Microsoft.BingImagery.Style of the imagery.</param>
        /// <param name="permit">The Pyxis.Publishing.Permits.ExternalApiKeyPermit for Microsoft Bing imagery to use when requesting the Microsoft Bing imagery.</param>
        /// <returns>The created Pyxis.Contract.Publishing.GeoSource.</returns>
        public GeoSource Create(Style style, ExternalApiKeyPermit permit)
        {
            //convert secure string into normal string
            var bingKey = new System.Net.NetworkCredential(null, permit.Key).Password;

            return Create(style, bingKey );
        }

        /// <summary>
        /// Create a Pyxis.Contract.Publishing.GeoSource for Microsoft Bing Imagery of the specified style.
        /// </summary>
        /// <param name="style">The desired Pyxis.IO.Microsoft.BingImagery.Style of the imagery.</param>
        /// <param name="key">The Microsoft Bing imagery key to use when requesting the Microsoft Bing imagery.</param>
        /// <returns>The created Pyxis.Contract.Publishing.GeoSource.</returns>
        public GeoSource Create(Style style, string key)
        {
            var bing = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo("{68EDBF04-72BD-4009-B388-834B8A6AE3C5}")
                .AddAttribute("MapType", style.ToString())
                .AddAttribute("Key", key)
                );

            var sampler = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.NearestNeighbourSampler)
                .AddInput(0,bing)
                );

            var process = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.CoverageCache)
                .AddInput(0, sampler)
                );

            var id = Guid.NewGuid();
            return new GeoSource(
                id: id,
                licenses: new List<LicenseReference>(),
                metadata: new Metadata(
                    name: "Microsoft Bing " + style.GetDescription(),
                    description: "",
                    user: Engine.GetUserInfo(),
                    providers: new List<Provider>(),
                    category: "",
                    tags: new List<string>(),
                    systemTags: new List<string>(),
                    created: DateTime.Now,
                    updated: DateTime.Now,
                    basedOnVersion: null,
                    externalUrls: new List<ExternalUrl>(),
                    visibility: VisibilityType.Private,
                    comments: new LinkedList<AggregateComment>(),
                    ratings: new AggregateRatings()),
                version: Guid.NewGuid(),
                procRef: pyxlib.procRefToStr(new ProcRef(process)),
                definition: PipeManager.writePipelineToNewString(process),
                basedOn: new List<ResourceReference>(),
                state: null,
                dataSize: 0,
                styles: new List<ResourceReference>(),
                usedBy: new List<Guid>(),
                related: new List<Guid>());
        }
    }

    internal static class EnumExtentations {
        public static string GetDescription(this Enum value)
        {
            var type = value.GetType();
            var field = type.GetField(value.ToString());
            var attributes = field.GetCustomAttributes(typeof(DescriptionAttribute), false);
            if (attributes.Length > 0) return (attributes[0] as DescriptionAttribute).Description;
            return value.ToString();
        }
    }
}
