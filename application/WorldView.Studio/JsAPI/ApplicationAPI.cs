using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Threading.Tasks;
using System.Windows.Forms;
using Pyxis.Contract.Converters;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Import;
using Pyxis.IO.Sources.Local;
using Pyxis.Publishing.Permits;
using Pyxis.Utilities;
using Pyxis.WorldView.Studio.Layers;
using Pyxis.WorldView.Studio.Layers.Html;
using Pyxis.WorldView.Studio.Storage;

namespace Pyxis.WorldView.Studio.JsAPI
{
    internal class ApplicationAPI
    {
        private ApplicationForm Form { get; set; }

        /// <summary>
        /// A service for the embedded browser to persist data. This ObjectStore is a key,value storage on disk
        /// </summary>
        public ObjectStorage ObjectStorage { get; private set; }

        /// <summary>
        /// A service for the embedded browser to persist images (used for screen capture).
        /// </summary>
        public ImageStorage ImageStorage { get; private set; }


        public ApplicationAPI(ApplicationForm form)
        {
            Form = form;

            //create services
            var storageDirectory = Form.EngineConfig.WorkingDirectory + Path.DirectorySeparatorChar + Form.EngineConfig.EngineName + ".Storage";
            ObjectStorage = new ObjectStorage(storageDirectory);

            var imageStorageDirectory = Form.EngineConfig.WorkingDirectory + Path.DirectorySeparatorChar + Form.EngineConfig.EngineName + ".ImageStorage";
            ImageStorage = new ImageStorage(imageStorageDirectory);
        }

        public void Register(HtmlLayer HtmlLayer)
        {
            var uiScheduler = TaskScheduler.FromCurrentSynchronizationContext();

            HtmlLayer.RegisterProxy("application", proxy =>
            {
                proxy.BindAsync("bringToFront", () =>
                {
                    Form.InvokeIfRequired(Form.Activate);
                    return true;
                });

                proxy.BindAsync("load", (string name, JsonString defaultValue) =>
                {
                    // defaultValue may be null - avoid an exception
                    return new JsonString(ObjectStorage.Load(
                        name,
                        defaultValue != null ? defaultValue.ToString() : "")
                        );
                });

                proxy.BindAsync("save", (string name, JsonString value) =>
                {
                    ObjectStorage.Save(name, value.ToString());
                    return true;
                });

                proxy.BindAsync("close", () =>
                {
                    Form.BeginInvoke((MethodInvoker)(() =>
                    {
                        Form.Close();
                    }));
                    return true;
                });

                proxy.BindAsync("restart", () =>
                {
                    Form.BeginInvoke((MethodInvoker)(() =>
                    {
                        Form.Close();
                        Application.Restart();
                    }));
                    return true;
                });

                proxy.BindAsync("getToken", () =>
                {
                    var user = Form.Engine.GetUser();

                    if (user != null)
                    {
                        return GetTokenDetails(user.TokenRetainer.GetPermit());
                    }
                    else
                    {
                        return new AccessToken.TokenDetails();
                    }
                });

                proxy.BindAsync("login", (string username, string password) =>
                {
                    if (Form.EngineConfig.User != null)
                    {
                        throw new InvalidOperationException("Login can be done only once.");
                    }

                    var channel = Form.Engine.GetChannel();
                    var authChannel = channel.Authenticate(new NetworkCredential(username, password));

                    Form.AuthenticateEngine(authChannel.AsUser());

                    var tokenDetails = GetTokenDetails(authChannel.TokenRetainer.GetPermit());
                    return tokenDetails;
                });

                proxy.BindAsync("loginWithToken", (AccessToken.TokenDetails tokenDetails) =>
                {
                    if (Form.EngineConfig.User != null)
                    {
                        throw new InvalidOperationException("Login can be done only once.");
                    }
                    var channel = Form.Engine.GetChannel();
                    var authChannel = channel.Authenticate(tokenDetails);

                    Form.AuthenticateEngine(authChannel.AsUser());

                    return tokenDetails;
                });

                var fileDragEnter = proxy.Callback<string[]>("fileDragEnter");
                var fileDragLeave = proxy.Callback<string[]>("fileDragLeave");
                var fileDragDrop = proxy.Callback<DataSet[]>("fileDragDrop");

                Form.PyxisView.DragEnter += (s, e) =>
                {
                    var files = new string[] { "for backward compatibility" };
                    fileDragEnter(files);
                    e.Effect = DragDropEffects.Copy;
                };

                Form.PyxisView.DragLeave += (s, e) =>
                {
                    var files = new string[] { "for backward compatibility" };
                    fileDragLeave(files);
                };

                Form.PyxisView.DragDrop += (s, e) =>
                {
                    var dataSets = GetAllDataSets(e.Data);
                    if (dataSets != null)
                    {
                        fileDragDrop(dataSets);
                    }
                };

                proxy.BindAsync("openFileDialog", () =>
                {
                    var fileNames = new List<string>();
                    var dialogResult = DialogResult.None;

                    Form.InvokeIfRequired(() =>
                    {
                        var dialog = new OpenFileDialog();
                        dialog.Multiselect = true;
                        dialogResult = dialog.ShowDialog(Form.ParentForm);
                        if (dialogResult == DialogResult.OK)
                        {
                            fileNames = dialog.FileNames.ToList();
                        }
                    });

                    // Create data sets from the file paths
                    var dataSets = new List<DataSet>();
                    var localService = new LocalDataSetDiscoveryService();
                    foreach (var file in fileNames)
                    {
                        // First, create a catalog that contains the wanted data sets
                        try
                        {
                            var catalog = localService.BuildCatalog(file);
                            if (catalog != null)
                            {
                                // Then, retrieve all loadable data sets inside it
                                // Note: for files that contain layers, this will cause all layers to be presented separately
                                dataSets.AddRange(catalog.AllLoadableDataSets());
                            }
                        }
                        catch (Exception e)
                        {
                            Trace.error("Failed to create a data set from a file path: " + e.Message);
                        }
                    }

                    if (dialogResult == DialogResult.OK)
                    {
                        return dataSets;
                    }
                    else
                    {
                        throw new Exception("Open canceled");
                    }
                });

                var onCommandLine = proxy.Callback<string[]>("onCommandLine");

                Form.CommandLineParser.OnCommand += (s, e) =>
                {
                    Form.BeginInvoke((MethodInvoker)(() =>
                    {
                        //notify the javascript context about the command line
                        onCommandLine(e.Args);
                    }));
                };

                proxy.Bind("getNextCommandLine", () =>
                {
                    var commandLineEventArgs = Form.CommandLineQueue.TryGetNextCommand();
                    return commandLineEventArgs != null ? commandLineEventArgs.Args : new string[] { };
                });

                proxy.BindAsync("resourceToDataUrl", (string url) =>
                {
                    var bitmap = Image.FromFile(ImageStorage.FromResourceName(url));
                    return bitmap.ToDataUrl();
                });

                proxy.Bind("getVersion", () =>
                {
                    var version = Application.ProductVersion; //return real version or 0.0.0.0
                    var defaultVersion = new Version().ToString(); //return 0.0
                    return version.StartsWith(defaultVersion) ? "DEV" : version;
                });

                Feedback.Feedback feedback = null;

                proxy.Bind("captureApplicationState", () =>
                {
                    feedback = Feedback.Feedback.Create(Form);
                    return true;
                });

                proxy.BindAsync("sendFeedback", (string message, bool attachScreenshot, bool attachTrace) =>
                {
                    if (feedback != null)
                    {
                        feedback.Message = message;
                        feedback.AttachScreenshot = attachScreenshot;
                        feedback.AttachTrace = attachTrace;
                        feedback.Send();
                    }
                    return true;
                });
            });
        }

        private static AccessToken.TokenDetails GetTokenDetails(AccessToken token)
        {
            //AccessToken.Details is private - this is hack I'm not proud off
            var detailProperty = typeof(AccessToken)
                .GetProperty("Details", BindingFlags.NonPublic | BindingFlags.Instance);

            var tokenDetails = (AccessToken.TokenDetails)detailProperty.GetValue(token);
            return tokenDetails;
        }

        /// <summary>
        /// Retrieves catalogs of data sets associated with an IDataObject, presumably sent with
        /// a window event. Performs a recursive lookup if the object is associated with some
        /// folders.
        /// </summary>
        /// <param name="data">The object to inspect</param>
        /// <returns>A list of catalogs or null if the data object is not associated with any data sets</returns>
        private DataSet[] GetAllDataSets(IDataObject data)
        {
            // Check if the data is associated with file system items
            if (data != null && data.GetDataPresent(DataFormats.FileDrop))
            {
                // Get all file system items inside the data object
                var paths = data.GetData(DataFormats.FileDrop) as string[];
                if (paths != null)
                {
                    var result = new List<DataSet>();

                    foreach (var path in paths)
                    {
                        var catalog = Form.Engine.BuildCatalog(path);
                        if (catalog != null)
                        {
                            // only return loadable data sets
                            result.AddRange(catalog.AllLoadableDataSets());
                        }
                    }

                    return result.ToArray();
                }
            }

            return null;
        }
    }
}
