using System;
using System.Collections.Generic;
using System.Data;
using Pyxis.Utilities;

namespace PyxNet.Service
{
    // TODO: Refactor this class to achieve the following goals.
    // - Remove knowledge of the specific fact types, or use templating to make the code generic.
    // - Find certificates by passing in a fact.
    // - Index the repository by facts and only validate the matching certificates when finding them.
    /// <summary>
    /// Holds a "database" of certificates, including a number of "system"
    /// (read-only) certificates.
    /// </summary>
    public class CertificateRepository: IDisposable
    {
        #region Tracer

        public readonly NumberedTraceTool<CertificateRepository> Tracer =
            new NumberedTraceTool<CertificateRepository>(TraceTool.GlobalTraceLogEnabled);

        #endregion

        #region Fields

        /// <summary>The certificates are stored in a single table.</summary>
        private const string CertificateTableName = "Certificate";

        /// <summary>The database file name.</summary>
        private readonly string m_databaseFilename;

        /// <summary>The certificate repository.</summary>
        private readonly Certificates m_repository = new Certificates();
        private readonly object m_repositoryLock = new object();

        /// <summary>The system certificate repository.</summary>
        private static readonly CertificateRepository s_systemRepository =
            new CertificateRepository();

        /// <summary>An internal list of certificates.</summary>
        private DynamicList<Certificate> m_certificateList = null;
        private readonly object m_certificateListLock = new object();

        #endregion

        #region Properties

        /// <summary>
        /// Gets the system repository.
        /// </summary>
        /// <value>The system repository.</value>
        private static CertificateRepository SystemRepository
        {
            get { return CertificateRepository.s_systemRepository; }
        }

        /// <summary>
        /// Gets a value indicating whether this instance is the system repository.
        /// </summary>
        /// <value>
        /// 	<c>true</c> if this instance is system repository; otherwise, <c>false</c>.
        /// </value>
        private bool IsSystemRepository
        {
            get { return this == s_systemRepository; }
        }
        
        #endregion

        #region Construction and Finalization

        /// <summary>
        /// Initializes a new instance of the <see cref="CertificateRepository"/> class.
        /// </summary>
        /// <param name="databaseFilename">The database filename.</param>
        public CertificateRepository(string databaseFilename)
        {
            if ((s_systemRepository != null) && 
                (databaseFilename == s_systemRepository.m_databaseFilename))
            {
                throw new InvalidOperationException(String.Format(
                    "Invalid operation!  Attempt to access the system repository '{0}'.",
                    databaseFilename));
            }

            m_databaseFilename = databaseFilename;
            Tracer.DebugWriteLine("Database file name set to \"{0}\"", m_databaseFilename);

            DataTable table = m_repository.Tables[CertificateTableName];
            if (table == null)
            {
                throw new InvalidOperationException("Unable to find table in dataset.");
            }

            Tracer.DebugWriteLine("CertificateRepository: Does database file exist?");
            try
            {
                if (System.IO.File.Exists(m_databaseFilename))
                {
                    Tracer.DebugWriteLine("CertificateRepository: Database file \"{0}\" exists.", m_databaseFilename);

                    table.BeginLoadData();
                    m_repository.ReadXml(m_databaseFilename);
                    table.EndLoadData();

                    Tracer.DebugWriteLine("CertificateRepository: Database file read.");
                }
            }
            catch (Exception ex)
            {
                // TODO: Consider moving the file into an archive.  For now, 
                //  we just over-write it later.
                System.Diagnostics.Trace.WriteLine(string.Format(
                    "Repository \"{0}\" was unreadable and ignored. {1}.", 
                    m_databaseFilename, ex.Message));
            }
        }

        /// <summary>
        /// Initializes the system instance of the 
        /// <see cref="CertificateRepository"/> class.
        /// </summary>
        private CertificateRepository() : this("Certificate.XML")
        {
        }

        /// <summary>
        /// Destructor
        /// </summary>
        ~CertificateRepository()
        {
            Dispose(false);
        }

        #endregion

        #region IDisposable

        private bool m_disposed = false;

        /// <summary>
        /// Dispose of this object (as per IDisposable)
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Implementation of Dispose - will be called from Dispose or destructor.
        /// </summary>
        /// <remarks>Do NOT touch member variables if disposing is false!</remarks>
        /// <param name="disposing"></param>
        private void Dispose(bool disposing)
        {
            if (!this.m_disposed)
            {
                if (disposing)
                {
                    // TODO: Does nothing for now.  Soon we will do delayed writes.
                }
            }
            m_disposed = true;
        }

        #endregion

        #region Certificates

        /// <summary>
        /// Gets all certificates in the repository, plus any in the system repository.
        /// Removes any invalid certificates in the process.
        /// </summary>
        /// <value>The items.</value>
        public IEnumerable<Certificate> Certificates
        {
            get
            {
                foreach (Certificate certificate in Items)
                {
                    Certificate validCertificate = ValidateCertificate(certificate);
                    if (validCertificate != null)
                    {
                        yield return validCertificate;
                    }
                }
            }
        }

        /// <summary>
        /// Gets all certificates in the repository, plus any in the system repository.
        /// Does not validate.
        /// </summary>
        /// <value>The items.</value>
        private IEnumerable<Certificate> Items
        {
            get
            {
                lock (m_certificateListLock)
                {
                    if (m_certificateList == null)
                    {
                        DataRow[] rows;
                        lock (m_repositoryLock)
                        {
                            DataRowCollection rowCollection = m_repository.Certificate.Rows;
                            rows = new DataRow[rowCollection.Count];
                            rowCollection.CopyTo(rows, 0);
                        }

                        m_certificateList = new DynamicList<Certificate>();
                        foreach (DataRow row in rows)
                        {
                            Certificate certificate = ExtractCertificate(row);
                            if (certificate != null)
                            {
                                m_certificateList.Add(certificate);
                                yield return certificate;
                            }
                        }
                        if (!this.IsSystemRepository)
                        {
                            foreach (Certificate certificate in SystemRepository.Items)
                            {
                                m_certificateList.Add(certificate);
                                yield return certificate;
                            }
                        }
                    }
                    else
                    {
                        foreach (Certificate c in m_certificateList)
                        {
                            yield return c;
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Extracts the certificate from the row, handling any errors.
        /// </summary>
        /// <param name="row">The row.</param>
        /// <returns></returns>
        private static Certificate ExtractCertificate(DataRow row)
        {
            try
            {
                return new Certificate(row);
            }
            catch (Exception)
            {
                // The row might be bad.  Consider deleting it.
            }
            return null;
        }

        /// <summary>
        /// Validates the given certificate, and returns it iff it is valid.
        /// Otherwise a null is returned.  Automatically removes invalid
        /// certificates from the repository.
        /// </summary>
        /// <param name="certificate"></param>
        /// <returns>
        /// A validated certificate, or null (if the certificate was invalid.
        /// </returns>
        private Certificate ValidateCertificate(Certificate certificate)
        {
            if (certificate != null)
            {
                if (certificate.Valid)
                {
                    return certificate;
                }
                this.Remove(certificate);
            }
            return null;
        }

        #endregion

        #region Table

        /// <summary>
        /// Builds a query for selecting the specified id.
        /// </summary>
        /// <param name="id">The id.</param>
        /// <returns>The resulting string query to pass to "Select".</returns>
        private String BuildQuery(ServiceInstanceId id)
        {
            return String.Format("Id = '{0}'", id.ToString());
        }

        /// <summary>
        /// Tests to see if this is a system certificate.  Throws an 
        /// InvalidOperationException if it is!
        /// </summary>
        /// <param name="certificate">The certificate.</param>
        /// <param name="operation">The operation.</param>
        private void TestForSystemCertificate(Certificate certificate, string operation)
        {
            if (null == certificate)
            {
                throw new ArgumentNullException("certificate");
            }

            if (IsSystemRepository)
            {
                throw new InvalidOperationException(String.Format(
                    "Unable to {1} certificate '{0}' in system repository.",
                    certificate.ServiceInstance.ServiceInstanceId.ToString(), operation));
            }

            if (certificate.ServiceInstance != null)
            {
                bool exists;
                lock (m_repositoryLock)
                {
                    exists = (m_repository.Tables[CertificateTableName].Select(
                        BuildQuery(certificate.ServiceInstance.ServiceInstanceId)).Length > 0);
                }

                if (exists)
                {
                    throw new InvalidOperationException(String.Format(
                        "Certificate '{0}' exists in system repository.  Unable to {1} it.",
                        certificate.ServiceInstance.ServiceInstanceId.ToString(), operation));
                }
            }
        }

        /// <summary>
        /// Adds the specified new certificate.
        /// Throws if it is already in the system repository.
        /// </summary>
        /// <param name="newCertificate">The new certificate.</param>
        public void Add(Certificate newCertificate)
        {
            TestForSystemCertificate(newCertificate, "add");
            if (!newCertificate.Valid)
            {
                return;
            }

            lock (m_repositoryLock)
            {
                DataTable table = m_repository.Tables[CertificateTableName];
                DataRow newRow = table.NewRow();
                newCertificate.ToRow(newRow);

                DataRow oldRow = table.Rows.Find(newRow[Certificate.IndexColumnName]);
                if (oldRow != null)
                {
                    if (newRow[Certificate.ContentColumnName].Equals(oldRow[Certificate.ContentColumnName]))
                    {
                        return;
                    }
                    table.Rows.Remove(oldRow);
                }
                table.Rows.Add(newRow);
                Save();
            }
            
            lock (m_certificateListLock)
            {
	            if (m_certificateList != null)
	            {
	                m_certificateList.Add(newCertificate);
	            }
            }

            OnCertificateAdded(this, newCertificate);
        }

        // TODO: Make this more efficient, possibly by creating a thread that goes through and removes stuff.
        /// <summary>
        /// Removes the specified removed certificate.
        /// </summary>
        /// <param name="removedCertificate">The removed certificate.</param>
        public bool Remove(Certificate removedCertificate)
        {
            // We only remove from the local (non-system) repository.
            //TestForSystemCertificate(modifiedCertificate, "remove");
            bool found = false;

            lock (m_repositoryLock)
            {
                List<DataRow> rowsToRemove = new List<DataRow>();

                foreach (DataRow row in m_repository.Certificate.Rows)
                {
                    Certificate certificate = ExtractCertificate(row);
                    if (certificate == null)
                    {
                        rowsToRemove.Add(row);
                    }
                    else if (removedCertificate.Equals(certificate))
                    {
                        rowsToRemove.Add(row);
                        found = true;
                    }
                }

                DataTable table = m_repository.Tables[CertificateTableName];
                foreach (DataRow r in rowsToRemove)
                {
                    table.Rows.Remove(r);
                }

                if (rowsToRemove.Count > 0)
                {
                    Save();
                }
            }

            lock (m_certificateListLock)
            {
                if (m_certificateList != null)
                {
                    m_certificateList.Remove(removedCertificate);
                }
            }
            return found;
        }

        /// <summary>
        /// Saves the data to the database file.
        /// </summary>
        private void Save()
        {
        	try
        	{
        	    lock (m_repositoryLock)
        	    {
                	m_repository.WriteXml(m_databaseFilename);
                }
        	}
        	catch (System.IO.IOException e)
        	{
                // Failed to write.  We ignore for now.
                Tracer.DebugWrite(
                    String.Format("Failed to write to certificate repository: {0}", e.Message));
        	}
        }

        #endregion

        #region Resource Instance Facts

        /// <summary>
        /// Gets the resource instance facts.
        /// </summary>
        /// <param name="resourceId">The resource id.</param>
        /// <returns></returns>
        private IEnumerable<ResourceInstanceFact> GetResourceInstanceFacts(ResourceId resourceId)
        {
            return GetResourceInstanceFacts(
                delegate(ResourceInstanceFact fact)
                {
                    return fact.ResourceId == resourceId;
                });
        }

        /// <summary>
        /// Get resource instance facts for which the predicate holds true.
        /// </summary>
        /// <param name="include">
        /// A predicate that takes a resource instance fact and returns true if it is to be included.
        /// </param>
        /// <returns>Each fact that meets the "include" criteria.</returns>
        public IEnumerable<ResourceInstanceFact> GetResourceInstanceFacts(Predicate<ResourceInstanceFact> include)
        {
            foreach (Certificate certificate in Items)
            {
                foreach (ICertifiableFact fact in certificate.Facts)
                {
                    ResourceInstanceFact instanceFact = fact as ResourceInstanceFact;
                    if (instanceFact != null && include(instanceFact))
                    {
                        // Validate the certificate.
                        Certificate validCertificate = ValidateCertificate(certificate);
                        if (validCertificate != null)
                        {
                            yield return instanceFact;
                        }
                    }
                }
            }
        }

        #endregion

        #region Generic Facts

        /// <summary>
        /// Gets the generic facts.
        /// </summary>
        /// <param name="id">The id.</param>
        /// <returns></returns>
        private IEnumerable<ICertifiableFact> GetGenericFacts(Guid id)
        {
            return GetGenericFacts(
                delegate(ICertifiableFact fact)
                {
                    return fact.Id == id;
                });
        }

        /// <summary>
        /// Gets the generic facts.
        /// </summary>
        /// <param name="include">
        /// A predicate that takes a certifiable fact and returns true if it is to be included.
        /// </param>
        /// <returns>Each fact that meets the "include" criteria.</returns>
        public IEnumerable<ICertifiableFact> GetGenericFacts(Predicate<ICertifiableFact> include)
        {
            foreach (Certificate certificate in Items)
            {
                foreach (ICertifiableFact fact in certificate.Facts)
                {
                    if (include(fact))
                    {
                        // Validate the certificate.
                        Certificate validCertificate = ValidateCertificate(certificate);
                        if (validCertificate != null)
                        {
                            yield return fact;
                        }
                    }
                }
            }
        }

        #endregion

        #region Service Instance Facts

        /// <summary>
        /// Gets the <see cref="System.Collections.Generic.IEnumerable&lt;PyxNet.Service.ServiceInstanceFact&gt;"/> 
        /// with the specified service id.
        /// </summary>
        /// <value></value>
        public IEnumerable<ServiceInstanceFact> this[ServiceId serviceId]
        {
            get
            {
                return GetServiceInstanceFacts(serviceId);
            }
        }

        /// <summary>
        /// Gets a single fact from a larger collection, or null for an empty collection.
        /// </summary>
        /// <param name="facts">The facts.</param>
        /// <returns></returns>
        private static ServiceInstanceFact TryGetFact(IEnumerable<ServiceInstanceFact> facts)
        {
            foreach (ServiceInstanceFact fact in facts)
            {
                return fact;
            }
            return null;
        }

        /// <summary>
        /// Gets all the service instance facts.
        /// </summary>
        /// <param name="include">
        /// A predicate that takes a service instance fact and returns true if it is to be included.
        /// </param>
        /// <returns>Each fact that meets the "include" criteria.</returns>
        private IEnumerable<ServiceInstanceFact> GetServiceInstanceFacts(Predicate<ServiceInstanceFact> include)
        {
            foreach (Certificate certificate in Items)
            {
                foreach (ICertifiableFact fact in certificate.Facts)
                {
                    ServiceInstanceFact instanceFact = fact as ServiceInstanceFact;
                    if (instanceFact != null && include(instanceFact))
                    {
                        // Validate the certificate.
                        Certificate validCertificate = ValidateCertificate(certificate);
                        if (validCertificate != null)
                        {
                            yield return instanceFact;
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Gets all valid ServiceInstanceFacts for the specified service instance id.
        /// </summary>
        /// <param name="serviceInstanceId">The service instance id.</param>
        /// <returns></returns>
        private IEnumerable<ServiceInstanceFact> GetServiceInstanceFacts(ServiceInstanceId serviceInstanceId)
        {
            return GetServiceInstanceFacts(
                delegate(ServiceInstanceFact fact)
                {
                    return fact.ServiceInstance.Id == serviceInstanceId;
                });
        }

        /// <summary>
        /// Gets a valid ServiceInstanceFact for the specified service instance 
        /// id, or null if the instance can't be found.
        /// </summary>
        /// <param name="serviceInstanceId">The service instance id.</param>
        /// <returns></returns>
        public ServiceInstanceFact GetServiceInstanceFact(ServiceInstanceId serviceInstanceId)
        {
            return CertificateRepository.TryGetFact(GetServiceInstanceFacts(serviceInstanceId));
        }

        /// <summary>
        /// Gets all valid ServiceInstanceFacts for the specified service id. (Or null if none exist.)
        /// </summary>
        /// <param name="serviceId">The service id.</param>
        /// <returns></returns>
        private IEnumerable<ServiceInstanceFact> GetServiceInstanceFacts(ServiceId serviceId)
        {
            return GetServiceInstanceFacts(
                delegate(ServiceInstanceFact fact)
                {
                    return fact.ServiceInstance.ServiceId == serviceId;
                });
        }

        /// <summary>
        /// Get a valid ServiceInstanceFacts for the specified service id.  (Or null if none exist.)
        /// </summary>
        /// <param name="serviceId">The service id.</param>
        /// <returns></returns>
        public ServiceInstanceFact GetServiceInstanceFact(ServiceId serviceId)
        {
            return CertificateRepository.TryGetFact(GetServiceInstanceFacts(serviceId));
        }

        /// <summary>
        /// Requests a certificate for the given service id.  If none is found
        /// locally, then the network (PyxNet) is queried for a certificate.  
        /// This in turn may result in a certificate authority creating the 
        /// certificate.
        /// </summary>
        /// <param name="stack">The stack.</param>
        /// <param name="requestedServiceId">The requested service id.</param>
        /// <returns></returns>
        public Certificate RequestCertificate(
            Stack stack, ServiceId requestedServiceId)
        {
            Certificate result = null;

            //--
            //-- get all facts for service id, and find a valid one 
            //-- that matches the local node id
            //--
            IEnumerable<ServiceInstanceFact> factList = this.GetServiceInstanceFacts(requestedServiceId);
            if (factList != null)
            {
                foreach (ServiceInstanceFact fact in factList)
                {
                    if (fact.ServiceInstance.Server == stack.NodeInfo.NodeId)
                    {
                        if (fact.Certificate.Valid)
                        {
                            result = fact.Certificate;
                            break;
                        }
                    }
                }
            }

            //--
            //-- either we have not certificate or a valid certificate.
            //-- should not get an expired/invalid certificate here
            //--
            System.Diagnostics.Trace.Assert(result == null || result.Valid);

            if (result == null )
            {
                //--
                //-- build a new certificate here.
                //--

#if LICENSE_SERVER
                Tracer.DebugWriteLine(
                    "Couldn't get certificate locally.  Using requester...");

                // TODO: Add a way to just get the nodes that can issue the certificate, so we can specify who to contact.

                // Create service instance fact.
                // TODO: Fix. This is so wrong!
                ServiceInstanceFact serviceInstanceFact = new ServiceInstanceFact(
                    ServiceInstance.Create(requestedServiceId, stack.NodeInfo.NodeId));

                // Set up a timer to wait for the requester to finish, or timeout.
                Utilities.SynchronizationEvent certificateTimer =
                    new Utilities.SynchronizationEvent(TimeSpan.FromSeconds(15));

                // We don't have one, so run the query.
                CertificateRequester requester = new CertificateRequester(
                    stack, serviceInstanceFact);

                requester.DisplayUri +=
                    delegate(object sender, CertificateRequester.DisplayUriEventArgs e)
                    {
                        System.Diagnostics.Debug.WriteLine("A URI shouldn't be required when requesting a service instance fact.");
                    };

                requester.CertificateReceived += delegate(object sender, CertificateRequester.CertificateReceivedEventArgs e)
                {
                    result = e.Certificate;// stack.CertificateRepository.GetServiceInstanceFact(requestedServiceId);
                    this.Add(result);
                    certificateTimer.Pulse();
                };

                try
                {
                    requester.Start(TimeSpan.FromSeconds(30));
                }
                catch (ApplicationException ex)
                {
                    System.Diagnostics.Trace.WriteLine(string.Format(
                        "Unable to request a certificate: {0}", ex.Message));
                }

                // Wait for the requester to finish 
                // (it may have already, in which case this will return immediately).
                certificateTimer.Wait();

                requester.Close();
#else
                Tracer.DebugWriteLine("CertificateRepository: Couldn't get certificate.  Forging...");

                ServiceInstance serviceInstance = ServiceInstance.Create(
                    requestedServiceId, stack.NodeInfo.NodeId);
                Certificate forgery = ServiceInstanceCertificateHelper.Create(stack.PrivateKey,
                    serviceInstance, // Any authority will do.
                    DateTime.Now + TimeSpan.FromDays(1), 
                    serviceInstance);
                this.Add(forgery);
                result = forgery;
#endif
            }

			if (null == result)
			{
	            Tracer.DebugWriteLine("CertificateRepository: Couldn't get certificate.");
			}
			else
			{
    	        Tracer.DebugWriteLine("CertificateRepository: Got a certificate!");
        	}

            return result;
        }

        #endregion

        #region CertificateAdded Event

        /// <summary> EventArgs for a CertificateAdded event. </summary>    
        public class CertificateAddedEventArgs : EventArgs
        {
            private Certificate m_Certificate;

            /// <summary>The Certificate.</summary>
            public Certificate Certificate
            {
                get { return m_Certificate; }
                set { m_Certificate = value; }
            }

            internal CertificateAddedEventArgs(Certificate theCertificate)
            {
                m_Certificate = theCertificate;
            }
        }

        /// <summary> Event handler for CertificateAdded. </summary>
        public event EventHandler<CertificateAddedEventArgs> CertificateAdded
        {
            add
            {
                m_CertificateAdded.Add(value);
            }
            remove
            {
                m_CertificateAdded.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<CertificateAddedEventArgs> m_CertificateAdded = new Pyxis.Utilities.EventHelper<CertificateAddedEventArgs>();

        /// <summary>
        /// Raises the CertificateAdded event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theCertificate"></param>
        private void OnCertificateAdded(object sender, Certificate theCertificate)
        {
            m_CertificateAdded.Invoke( sender, new CertificateAddedEventArgs(theCertificate));
        }

        #endregion CertificateAdded Event

        #region Get Matching Facts

        /// <summary>
        /// Gets the matching facts for the given "query".
        /// </summary>
        /// <param name="query">The query.</param>
        /// <param name="factType">The type of fact you want to get.</param>
        /// <returns></returns>
        public IEnumerable<ICertifiableFact> GetMatchingFacts(string searchTerm, Type factType)
        {
            foreach (ICertifiableFact fact in this.GetMatchingFacts(searchTerm))
            {
                if (fact.GetType() == factType)
                {
                    yield return fact;
                }
            }
        }

        /// <summary>
        /// Gets the matching facts for the given "query".
        /// </summary>
        /// <param name="query">The query.</param>
        /// <returns></returns>
        public IEnumerable<ICertifiableFact> GetMatchingFacts(string searchTerm)
        {
            return GetGenericFacts(
                delegate(ICertifiableFact fact)
                {
                    foreach (String keyword in fact.Keywords)
                    {
                        if (keyword == searchTerm)
                        {
                            return true;
                        }
                    }
                    return false;
                });
        }

        #endregion
    }
}

