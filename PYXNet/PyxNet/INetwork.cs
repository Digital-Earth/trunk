using System;

namespace PyxNet
{
    /// <summary>
    /// EventArgs for connection opened event (on a network.)
    /// </summary>
    public class ConnectionEventArgs : EventArgs
    {
        public INetworkConnection Connection { get; set; }
    }

    /// <summary>
    /// An INetworkConnection factory.
    /// </summary>
    public interface INetwork
    {
        Pyxis.Utilities.NumberedTraceTool<INetwork> Tracer
        {
            get;
        }

        /// <summary>
        /// Removes all listeners.
        /// </summary>
        void RemoveListeners();

        /// <summary>
        /// Creates and adds listeners for the address.
        /// </summary>
        /// <param name="localAddress">The address to listen on.</param>
        void SetListeners(NetworkAddress address);

        /// <summary>
        /// Create a new connection to the network.
        /// </summary>
        /// <param name="address">The address of the computer to connect to.</param>
        /// <returns>The new network connection.</returns>
        INetworkConnection Connect(NetworkAddress address);

        /// <summary>
        /// Create a new connection to the network, from a specific address.
        /// </summary>
        /// <param name="to">The address of the computer to connect to.</param>
        /// <param name="from">The address of the computer to connect from.</param>
        /// <returns>The new network connection.</returns>
        INetworkConnection Connect(NetworkAddress to, NetworkAddress from);

        /// <summary>
        /// Callback that fires when a new connection is made to the network.
        /// </summary>
        event EventHandler<ConnectionEventArgs> ConnectionOpened;
    }
}