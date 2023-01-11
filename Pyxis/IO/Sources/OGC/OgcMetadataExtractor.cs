using System.Collections.Generic;
using System.Linq;
using System.Xml;
using ApplicationUtility;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// XmlNodeExtractor that extract metadata information using OGC common descriptors
    /// </summary>
    internal class OgcMetadataExtractor : XmlNodeExtractor
    {
        private readonly SimpleMetadata m_metadata;

        public OgcMetadataExtractor(SimpleMetadata metadata)
        {
            m_metadata = metadata;
            On("title", (node) => m_metadata.Name = node.InnerText.Trim());
            On("abstract", (node) => m_metadata.Description = node.InnerText.Trim());
            On("keywordlist", PopulateTags);
            On("keywords", PopulateTags);
            
            //description node can have title,abstract and other under that node.
            On("description", base.Parse);
        }

        /// <summary>
        /// Some OGC servers like to use comma separated inside the keywords. some use one tag per Keyword xml element.
        /// </summary>
        /// <param name="tagOrMultiTags">string that looks like tag</param>
        /// <returns>all tags extracted from the string</returns>
        private static IEnumerable<string> ExtractTagsFromString(string tagOrMultiTags)
        {
            //detect "tagA, tagB, tagC" case
            if (tagOrMultiTags.Contains(','))
            {
                return tagOrMultiTags.Split(',').Select(x => x.Trim()).Where(x => x.HasContent());
            }
            //detect "tagA tagB tagC" case
            else if (tagOrMultiTags.Contains(' '))
            {
                return tagOrMultiTags.Split(' ').Select(x => x.Trim()).Where(x => x.HasContent());
            }
            else
            {
                return new[] {tagOrMultiTags};
            }
        }

        public void PopulateTags(XmlNode keywords)
        {
            if (m_metadata.Tags == null)
            {
                m_metadata.Tags = new List<string>();
            }

            m_metadata.Tags.AddRange(
                keywords.ChildNodes.Cast<XmlNode>()
                    .Select(keyword => keyword.InnerText.Trim())
                    .Where(tag => !string.IsNullOrEmpty(tag)));

            //in case we extracted only one tag, check if this server have one big Keyword tag with many tags
            if (m_metadata.Tags.Count == 1)
            {
                m_metadata.Tags = ExtractTagsFromString(m_metadata.Tags[0]).ToList();
            }

            //clear tags array if no tags were collected
            if (m_metadata.Tags != null && m_metadata.Tags.Count == 0)
            {
                m_metadata.Tags = null;
            }
        }
    }
}
