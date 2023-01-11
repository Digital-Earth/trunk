using Awesomium.Core.Data;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.WorldView.Studio.Layers.Html
{
    /// <summary>
    /// Interface IAssetProvider is used to attached custom asset providers to the HtmlLayer.
    /// </summary>
    public interface IAssetProvider
    {
        /// <summary>
        /// Gets the name of the asset provider. the asset provider would receive DataSourceRequests in the form of asset://[name]/...
        /// </summary>
        string Name { get; }

        /// <summary>
        /// the Provide method would be invoked on a every request to a asset provider
        /// </summary>
        /// <param name="request">the request</param>
        /// <returns>async response to the request</returns>
        Task<AssetResponse> Provide(DataSourceRequest request);
    }

    /// <summary>
    /// AssetResponse represent an asset response.
    /// </summary>
    public class AssetResponse
    {
        /// <summary>
        /// Gets or sets the Data of the response.
        /// </summary>
        public byte[] Data { get; set; }

        /// <summary>
        /// Gets or sets the MimeType of the response
        /// </summary>
        public string MimeType { get; set; }
    }    

    /// <summary>
    /// Helper class to convert IAssetProvider into Awesomium.Core.Data.DataSource
    /// </summary>
    public class DataSourceWrapper : DataSource
    {
        public IAssetProvider Provider { get; private set; }
        
        public DataSourceWrapper(IAssetProvider provider)
        {
            Provider = provider;
        }

        protected override void OnRequest(DataSourceRequest request)
        {
            Provider.Provide(request).ContinueWith((task) =>
            {
                if (task.IsCompleted && task.Result.Data != null)
                {
                    IntPtr unmanagedPointer = IntPtr.Zero;
                    try
                    {
                        var buffer = task.Result.Data;
                        unmanagedPointer = System.Runtime.InteropServices.Marshal.AllocHGlobal(buffer.Length);
                        System.Runtime.InteropServices.Marshal.Copy(buffer, 0, unmanagedPointer, buffer.Length);

                        SendResponse(request, new DataSourceResponse()
                        {
                            Buffer = unmanagedPointer,
                            Size = (uint)buffer.Length,
                            MimeType = task.Result.MimeType
                        });
                    }
                    finally
                    {
                        if (unmanagedPointer != IntPtr.Zero)
                        {
                            System.Runtime.InteropServices.Marshal.FreeHGlobal(unmanagedPointer);
                            unmanagedPointer = IntPtr.Zero;
                        }
                    }                    
                }
                else
                {
                    SendRequestFailed(request);
                }
            });
        }        
    } 
}
