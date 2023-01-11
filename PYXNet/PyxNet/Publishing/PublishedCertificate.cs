using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Publishing
{
    // TODO: This file has never been reviewed.  Ever.
    /// <summary>
    /// A single certificate, all wrapped up for publishing.
    /// </summary>
    public class PublishedCertificate : Publisher.IPublishedItemInfo
    {
        private Service.Certificate m_certificate;

        /// <summary>
        /// Gets or sets the certificate.
        /// </summary>
        /// <value>The certificate.</value>
        public Service.Certificate Certificate
        {
            get { return m_certificate; }
            set { m_certificate = value; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="PublishedCertificate"/> class.
        /// </summary>
        /// <param name="certificate">The certificate.</param>
        public PublishedCertificate(Service.Certificate certificate)
        {
            m_certificate = certificate;
        }

        #region IPublishedItemInfo Members

        /// <summary>
        /// Gets the id.
        /// </summary>
        /// <value>The id.</value>
        public Guid Id
        {
            get { return Certificate.ServiceInstance.ServiceInstanceId.Guid; }
        }

        /// <summary>
        /// Gets the keywords.
        /// </summary>
        /// <value>A list of keywords.</value>
        public IEnumerable<string> Keywords
        {
            get
            {
                foreach (PyxNet.Service.ICertifiableFact fact in Certificate.Facts)
                {
                    foreach (string keyword in fact.Keywords)
                    {
                        yield return keyword;
                    }
                }
            }
        }

        /// <summary>
        /// Does this match the specified query?
        /// </summary>
        /// <param name="query">The query.</param>
        /// <param name="stack">The stack.</param>
        /// <returns>
        /// True iff this matches the specified query.
        /// </returns>
        public QueryResult Matches(Query query, Stack stack)
        {
            foreach (string keyword in Keywords)
            {
                if (query.Contents.Contains(keyword))
                {
                    return Publisher.CreateQueryResult( stack, query, Certificate.ToMessage());
                }
            }
            return null;
        }
        #endregion
    }

}
