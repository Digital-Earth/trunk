namespace Pyxis.Publishing
{
    /// <summary>
    /// Provides access to the valid license server urls.
    /// </summary>
    public class ApiUrl
    {
        /// <summary>
        /// The production license server rest API.
        /// </summary>
        static public string ProductionLicenseServerRestAPI
        {
            get { return Properties.Settings.Default.ProductionLicenseServerRestAPI; }
        }

        /// <summary>
        /// The test license server rest API.
        /// </summary>
        static public string TestLicenseServerRestAPI
        {
            get { return Properties.Settings.Default.TestLicenseServerRestAPI; }
        }

        /// <summary>
        /// The development license server rest API.
        /// </summary>
        static public string DevelopmentLicenseServerRestAPI
        {
            get { return Properties.Settings.Default.DevelopmentLicenseServerRestAPI; }
        }
    }
}
