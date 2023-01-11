/******************************************************************************
CertificatePublisher.cs

begin      : 06/11/2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

namespace PyxNet.Publishing
{
    /// <summary>
    /// A Class for publishing Certificates over Pyxnet.
    /// </summary>
    internal class CertificatePublisher
    {
        private Publisher m_publisher;

        private Publisher Publisher
        {
            get { return m_publisher; }
        }

        public CertificatePublisher(Stack stack)
        {
            m_publisher = stack.Publisher;
        }

        public void PublishItem(PyxNet.Service.Certificate certificate)
        {
            this.Publisher.PublishItem(new PublishedCertificate(certificate));
        }

        public void PublishItem(PyxNet.Service.CertificateRepository repository)
        {
            if (null == repository)
            {
                return;
            }

            System.Threading.Thread thread = new System.Threading.Thread(
                delegate()
                {
                    // Publish any future certificates added to the repository.
                    repository.CertificateAdded += repository_CertificateAdded;

                    // Iterate through certificates in the repository and add them.
                    // This validates every certificate, which could take a long time, so it is on another thread.
                    foreach (PyxNet.Service.Certificate certificate in repository.Certificates)
                    {
                        this.PublishItem(certificate);
                    }
                });
            thread.Name = "CertificatePublisher Publishing Thread";
            thread.IsBackground = true;
            thread.Start();
        }

        private void repository_CertificateAdded(object sender, PyxNet.Service.CertificateRepository.CertificateAddedEventArgs e)
        {
            this.PublishItem(e.Certificate);
        }
    }
}