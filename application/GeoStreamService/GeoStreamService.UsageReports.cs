/*****************************************************************************
UsageReports.cs

begin		: May 13, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using HoytSoft.Common.Services;
using Pyxis.Services.PipelineLibrary.Domain;
using Pyxis.Services.PipelineLibrary.Repositories;
using Pyxis.Utilities;
using System;
using System.Collections.Generic;
using System.Data;

namespace GeoStreamService
{
    public partial class GeoStreamService : ServiceBase
    {
        //--
        //-- message handler for messages sent from license server to geostream server
        //-- regarding usage reports.
        //--


        #region usage reports message handlers

        private void OnUsageReportsMessage(object sender, PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            PyxNet.Publishing.UsageReportsMessage reportMessage = new PyxNet.Publishing.UsageReportsMessage(args.Message);
            switch (reportMessage.Mode)
            {
                case PyxNet.Publishing.UsageReportsMessage.TransferMode.ReceiptAck:
                    Pyxis.Utilities.UsageReports.AcknowledgeReport(reportMessage.ReportFileName);
                    break;

                case PyxNet.Publishing.UsageReportsMessage.TransferMode.ResendRequest:
                    Pyxis.Utilities.UsageReports.ResendReport(reportMessage.ReportFileName);
                    break;

                default:
                    Pyxis.Utilities.UsageReports.Log.Warning("Unknown (or unexpected) usage report message");
                    break;
            }
        }

        #endregion usage reports message handlers

        //------------------------------------------------------------------------
        //--
        //-- USAGE REPORTS HELPER CLASS
        //--
        //------------------------------------------------------------------------

        /// <summary>
        /// Usage Reports Helper class.
        ///
        /// The Usage Reports module is restricted in its knowledge of the outside world.
        /// It does not understand PyxNet or other related topics.
        /// The usage reports utility calls this helper class which the carries out
        /// these operations for it.
        /// </summary>
        public class UsageReportsHelper : Pyxis.Utilities.UsageReportsHelper
        {
            /// <summary>
            /// Finds the license server for a data source.
            /// </summary>
            /// <param name="dataSetId">Data Source ID.</param>
            /// <returns></returns>
            public override Guid FindLicenseServer(Guid dataSetId)
            {
                PyxNet.Stack stack = PyxNet.StackSingleton.Stack;
                stack.Tracer.Enabled = true;

                //--
                //-- Build published pipeline fact for the data set that we want to use.
                //--
                PyxNet.Service.ResourceId resourceId = new PyxNet.Service.ResourceId(dataSetId);

                PyxNet.Service.PublishedPipelineFact pipelineFact =
                    new PyxNet.Service.PublishedPipelineFact();
                pipelineFact.Id = resourceId;
                DataSourceInfo dataSrcInfo = GetDataSourceInfo(dataSetId);
                if (dataSrcInfo != null)
                {
                    pipelineFact.Name = dataSrcInfo.Name;
                    pipelineFact.Description = dataSrcInfo.Desc;
                }
                else
                {
                    pipelineFact.Name = dataSetId.ToString();
                    pipelineFact.Description = dataSetId.ToString();
                }
                pipelineFact.Metadata = "data set metadata";
                pipelineFact.Certificate = null;
                // TODO: Fill in XML for pipeline fact.

                //--
                //-- Find certificate, search stack
                //--
                PyxNet.Service.CertificateFinder finder = new PyxNet.Service.CertificateFinder(stack, pipelineFact);
                PyxNet.Service.ICertifiableFact resourcePermissionFact = finder.Find();

                PyxNet.Service.Certificate certificate = null;
                if (resourcePermissionFact == null)
                {
                    //--
                    //-- no certificate found!
                    //--
                    Log.Info("UsageReports:FindLicenseServer: No Certificate (DataSet={0})", dataSetId.ToString());
                    Log.Info("UsageReports:FindLicenseServer: CertificateAuthorityServiceId = {0}",
                        PyxNet.Service.CertificateServer.CertificateAuthorityServiceId);

                    //--
                    //-- Find a license server.
                    //--
                    PyxNet.Service.ServiceFinder licenseServerFinder = new PyxNet.Service.ServiceFinder(stack);
                    PyxNet.Service.ServiceInstance licenseServer = licenseServerFinder.FindService(
                        PyxNet.Service.CertificateServer.CertificateAuthorityServiceId,
                        TimeSpan.FromSeconds(60));

                    if (licenseServer == null)
                    {
                        Log.Info("UsageReports:FindLicenseServer: No license server found.");
                    }

                    if (licenseServer != null)
                    {
                        SynchronizationEvent permissionGrantedTimer = new SynchronizationEvent(TimeSpan.FromSeconds(30));

                        //--
                        //-- have license server rubber stamp this request if it has to
                        //-- create a new certificate.
                        //--
                        pipelineFact.RubberStamp = true;

                        //--
                        //-- build certificate request, with permission fact
                        //--
                        PyxNet.Service.CertificateRequester requester =
                            new PyxNet.Service.CertificateRequester(stack, pipelineFact);

                        requester.DisplayUri +=
                            delegate(object sender, PyxNet.DisplayUriEventArgs a)
                            {
                                Log.Error("UsageReports:FindLicenseServer: DisplayUri - should never happen");
                            };

                        requester.CertificateReceived +=
                            delegate(object sender, PyxNet.Service.CertificateRequester.CertificateReceivedEventArgs c)
                            {
                                Log.Info("UsageReports:FindLicenseServer: CertificateReceived");

                                //--
                                //-- Get the certificate.
                                //--
                                certificate = c.Certificate;
                                System.Diagnostics.Debug.Assert(certificate != null);

                                //--
                                //-- Add it to the stack's certificate repository.
                                //--
                                stack.CertificateRepository.Add(certificate);
                            };

                        requester.PermissionGranted +=
                            delegate(object sender, PyxNet.Service.CertificateRequester.ResponseReceivedEventArgs e)
                            {
                                Log.Info("UsageReports:FindLicenseServer: PermissionGranted");
                                permissionGrantedTimer.Pulse();
                            };

                        //--
                        //-- send request to license server
                        //--
                        requester.Start(licenseServer, TimeSpan.FromSeconds(15));

                        //--
                        //-- Wait until we get one.
                        //--
                        permissionGrantedTimer.Wait();
                    }
                }
                else
                {
                    certificate = resourcePermissionFact.Certificate;
                }

                Guid licenseServerId = Guid.Empty;
                if (certificate != null)
                {
                    licenseServerId = certificate.Authority.Server.Identity;
                }
                return licenseServerId;
            }

            /// <summary>
            /// Find Guid for local host, requires a PyxNet stack call.
            /// Called from within the UsageReports utility, that know nothing about PyxNet.
            /// </summary>
            /// <returns>Guid for local host.</returns>
            public override Guid FindLocalHost()
            {
                Guid localHost = PyxNet.StackSingleton.Stack.NodeInfo.NodeGUID;
                return localHost;
            }

            /// <summary>
            /// Finds the friendly name of a PyxNet node.
            /// Called from within the UsageReports utility, that know nothing about PyxNet.
            /// </summary>
            /// <param name="nodeId">Node ID.</param>
            /// <returns>Friendly name for node ID</returns>
            public override string FindNodeFriendlyName(Guid nodeId)
            {
                string friendlyName = null;

                try
                {
                    PyxNet.NodeId id = new PyxNet.NodeId(nodeId);

                    PyxNet.NodeInfo found = PyxNet.NodeInfo.Find(PyxNet.StackSingleton.Stack, id, TimeSpan.FromSeconds(15));
                    friendlyName = found.FriendlyName;
                }
                catch (Exception ex)
                {
                    Trace.info("FindNodeFriendlyName:Exception: " + ex.ToString());
                }

                if (string.IsNullOrEmpty(friendlyName))
                {
                    friendlyName = base.FindNodeFriendlyName(nodeId);
                }

                return friendlyName;
            }

            /// <summary>
            /// Critical section inside SendReport, keeps access to internal logic
            /// to one thread at a time.
            /// </summary>
            private static object m_sendReportCriticalSection = new Object();

            /// <summary>
            /// Send report, called from within UsageReports object which has no knowledge of the PyxNet.
            /// Convert report data to XML, package message, and send using PyxNet.
            /// </summary>
            /// <param name="dataSet">Report data.</param>
            /// <param name="toNodeId">Destination node ID.</param>
            /// <param name="fromNodeId">Source node ID.</param>
            public override void SendReport(DataSet dataSet, Guid toNodeId, Guid fromNodeId)
            {
                Log.Info("UsageReports:SendReport:");

                lock (m_sendReportCriticalSection)
                {
                    Log.Trace("[+]UsageReports:SendReport:");
                    try
                    {
                        PyxNet.Publishing.UsageReportsMessage reportMessage = new PyxNet.Publishing.UsageReportsMessage();

                        reportMessage.Mode = PyxNet.Publishing.UsageReportsMessage.TransferMode.ReportSend;
                        reportMessage.DstNode = toNodeId;
                        reportMessage.SrcNode = fromNodeId;
                        reportMessage.ReportFileName = null;
                        reportMessage.ReportXml = null;

                        DataTable reportTable = dataSet.Tables["ReportInfo"];

                        System.Diagnostics.Trace.Assert(reportTable.Rows.Count == 1, "There should be one (and only one) ReportInfo row.");
                        reportMessage.ReportFileName = reportTable.Rows[0]["FileName"].ToString();
                        reportMessage.ReportXml = dataSet.GetXml();

                        reportMessage.SendReport(PyxNet.StackSingleton.Stack);
                    }
                    catch (Exception ex)
                    {
                        Log.Error("UsageReports:SendReports:Exception: " + ex.Message);
                    }
                    Log.Trace("[-]UsageReports:SendReports:");
                }
            }

            public override DataSourceInfo GetDataSourceInfo(Guid dataSourceId)
            {
                IList<Pipeline> pipelines =
                    PipelineRepository.Instance.GetAllPublishedPipelines();
                foreach (Pipeline pipeline in pipelines)
                {
                    if (pipeline.PipelineGuid == dataSourceId)
                    {
                        DataSourceInfo dsInfo = new DataSourceInfo();
                        dsInfo.Id = dataSourceId;
                        dsInfo.Name = pipeline.Name;
                        dsInfo.Desc = pipeline.Description;
                        return dsInfo;
                    }
                }

                return null;
            }

            /// <summary>
            /// Route log messages to application trace log.
            /// </summary>
            /// <param name="level">Message severity level.</param>
            /// <param name="message">The message.</param>
            public override void Logger(LogHelper.LogLevel level, string message)
            {
                switch (level)
                {
                    case LogHelper.LogLevel.Info:
                        Trace.info(message);
                        break;

                    case LogHelper.LogLevel.Warning:
                        Trace.notify(message);
                        break;

                    case LogHelper.LogLevel.Error:
                        Trace.error(message);
                        break;

                    case LogHelper.LogLevel.Debug:
                        Trace.debug(message);
                        break;

                    case LogHelper.LogLevel.Verbose:
                    case LogHelper.LogLevel.TraceOnly:
                    default:
                        Trace.memory(message);
                        break;
                }
            }
        }
    }
}