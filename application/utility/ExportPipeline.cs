/******************************************************************************
ExportPipeline.cs

begin      : November 16, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace ApplicationUtility
{
    public static class ExportPipeline
    {

        #region Export to ppl file

        /// <summary>
        /// Export the pipeline in a ppl format to a location of the
        /// users choice. Only the process is written out. Files that
        /// required by the process will not be copied to the location.
        /// </summary>
        /// <param name="proc">
        /// The process to write to a file. It is not necessary for the
        /// process to be able to initialize.
        /// </param>
        /// <returns>
        /// true if the user followed through with the save operation.
        /// </returns>
        static public bool ExportToPipelineFile(IProcess_SPtr proc)
        {
            System.Windows.Forms.SaveFileDialog saveDialog = 
                new System.Windows.Forms.SaveFileDialog();

            saveDialog.DefaultExt = "ppl";
            saveDialog.Filter = Properties.Resources.PipelineExportSaveAsType 
                + "|*.ppl|All Files|*.*";
            
            // the default file name is the root process name
            saveDialog.FileName = proc.getProcName() + ".ppl";

            if (saveDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                string savePath =
                    saveDialog.FileName;
                if(!savePath.EndsWith(".ppl"))
                {
                    savePath += ".ppl";
                }

                Console.WriteLine("Save Path: " + savePath);
                PipeManager.writePipelineToFile(savePath, proc);
            }
            else
            {
                // the user cancelled out of the save 
                Trace.ui("User chose not to export the pipeline.");
                return false;
            }

            return true;
        }

        #endregion

        #region Export through GDAL

        /// <summary>
        /// Export the passed coverage data source to an imagery file.
        /// </summary>
        /// <param name="proc">
        /// The initialized ICoverage process to convert to an image.
        /// </param>
        /// <returns>False, if the user cancels out.</returns>
        static public bool ExportImageFile(IProcess_SPtr proc)
        {
            try
            {
                BackgroundWorker exportThread = new BackgroundWorker();
                exportThread.WorkerSupportsCancellation = true;
                exportThread.DoWork += new DoWorkEventHandler(ExportThread_DoWork);
                exportThread.RunWorkerCompleted += new RunWorkerCompletedEventHandler(
                    ExportThread_RunWorkerCompleted);

                // Get save path/file from user.
                System.Windows.Forms.SaveFileDialog saveDialog = new System.Windows.Forms.SaveFileDialog();
                saveDialog.Title = Properties.Resources.SaveExportDialogTitle;
                saveDialog.Filter = "JPEG files (*.jpg)|*.jpg|All Files (*.*)|*.*";
                if (saveDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    ExportInfo info = new ExportInfo();
                    info.m_exportPath = saveDialog.FileName;
                    info.m_proc = proc;
                    exportThread.RunWorkerAsync(info);
                }
                else
                {
                    return false;
                }
            }
            catch
            {
                Trace.error(Properties.Resources.CantExport);
                System.Windows.Forms.MessageBox.Show(
                    Properties.Resources.ExportErrorText,
                    Properties.Resources.ExportErrorTitle,
                    System.Windows.Forms.MessageBoxButtons.OK,
                    System.Windows.Forms.MessageBoxIcon.Error);
            }

            return true;
        }
        
        private class ExportInfo
        {
            public string m_exportPath;
            public string m_tempSavePath;
            public IProcess_SPtr m_proc;
        }

        /// <summary>
        /// The worker method for thread that exports.
        /// NOTE: exceptions are caught in the RunWorkerCompleted method.
        /// </summary>
        /// <param name="sender">The sender of the event.</param>
        /// <param name="e">The event arguments.</param>
        private static void ExportThread_DoWork(object sender, DoWorkEventArgs e)
        {
            ExportInfo info = e.Argument as ExportInfo;
            System.Diagnostics.Debug.Assert(info != null);

            // verify that the input proc can provide a coverage
            ICoverage_SPtr coverageProc = pyxlib.QueryInterface_ICoverage(info.m_proc.getOutput());
            if (coverageProc.get() == null)
            {
                throw new ArgumentException("Export to file needs a coverage");
            }

            // TODO: Verify the output type can be interpreted as colour.

            // Build the GDALFileConverter Process.

            info.m_tempSavePath = AppServices.makeTempFile(".jpg");

            IProcess_SPtr converterProc =
                PYXCOMFactory.CreateProcess(
                    new PYXCOMProcessCreateInfo("{7732B135-8BB8-47f7-99E4-6E039FDF5067}")
                    .AddAttribute("savePath", info.m_tempSavePath)
                    .AddInput(0, info.m_proc)
                    );

            converterProc.initProc();

            // Convert the data and write to file.
            IDataProcessor_SPtr dataProc = pyxlib.QueryInterface_IDataProcessor(pyxlib.QueryInterface_PYXCOM_IUnknown(converterProc));
            dataProc.processData();

            e.Result = info;
        }

        /// <summary>
        /// Method that executes when the thread completes.
        /// </summary>
        /// <param name="sender">The sender of the event.</param>
        /// <param name="e">The event's arguments.</param>
        private static void ExportThread_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Error != null)
            {
                System.Windows.Forms.MessageBox.Show(
                    Properties.Resources.ExportErrorText,
                    Properties.Resources.ExportErrorTitle,
                    System.Windows.Forms.MessageBoxButtons.OK,
                    System.Windows.Forms.MessageBoxIcon.Error);
                Trace.error("An error occurred while exporting an image."
                    + e.Error.ToString());
            }
            else
            {
                // Copy temp export file to user selected save location.
                ExportInfo info = e.Result as ExportInfo;
                System.Diagnostics.Debug.Assert(info != null);

                System.IO.File.Copy(info.m_tempSavePath, info.m_exportPath);
                System.Windows.Forms.MessageBox.Show(
                    Properties.Resources.ExportCompleteText + ":\n " + info.m_exportPath,
                    Properties.Resources.ExportCompleteTitle,
                    System.Windows.Forms.MessageBoxButtons.OK,
                    System.Windows.Forms.MessageBoxIcon.Information);
            }
        }

        #endregion

    }
}
