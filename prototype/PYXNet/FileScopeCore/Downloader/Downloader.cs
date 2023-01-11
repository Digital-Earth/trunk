// Downloader.cs
// Copyright (C) 2002 Matt Zyzik (www.FileScope.com)
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

using System;
using System.Timers;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Collections;
using System.Runtime.Serialization;

namespace FileScopeCore
{
    /// <summary>
    /// Various states of the downloader.
    /// </summary>
    public enum DLState
    {
        Waiting,			//waiting to reconnect or waiting for something else
        Connecting,			//in the process of connecting
        SentPush,			//sent push request to firewalled host
        Connected,			//connected; but transfer hasn't started
        Downloading,		//transferring a file
        CouldNotConnect		//this downloader instance is done: we couldn't connect
    }

    /// <summary>
    /// The core download class that carries out the transfer.
    /// </summary>
    [Serializable]
    public class Downloader : IDeserializationCallback
    {
        //identifier for this downloader
        public int dlNum;

        //keep reference to the index of the parent DownloadManager
        public int dmNumParent;

        //stores the QueryHitObject associated with this instance
        public QueryHitObject qho = null;

        //connection timer
        [NonSerialized]
        public GoodTimer reConnect = new GoodTimer();

        public DLState state;
        public int count = 0;
        public uint startOffset = 0;
        public uint curOffset = 0;

        [NonSerialized]
        Socket sock1;

        //receive buffer
        public byte[] receiveBuff = null;

        //10 seconds to connect to host
        [NonSerialized]
        public GoodTimer connectYet = new GoodTimer();

        ArrayList tempBuf = new ArrayList();

        //timer disconnects from a connection that isn't responding
        [NonSerialized]
        public GoodTimer deadYet = new GoodTimer();

        //used in the above timer
        bool receivedData = false;

        //used to indicate handshake was received already
        public bool dlflushed = false;

        //if the network supports partial file downloading, this will be used a lot
        SortedList fileParts = new SortedList();

        //last message received (ex. "Server Busy", "File Not Found")
        public string lastMessage = "";

        public string chatIP = "";

        //store any other qhos that match this file; replaces original qho for networks not supporting swarm downloading
        public ArrayList otroQHOs = new ArrayList();

        //flag used for quick reconnects
        public bool quickReconnect = false;

        //keep track of queue state
        public int queueState = 0;

        //Disconnect could be fired multiple times; we'll use a flag to be safe
        private volatile bool disconnectFired = false;

        public Downloader(QueryHitObject qho, int dlNum, int dmNumParent, int knownDLcount)
        {
            //copy the QueryHitObject and identifier
            this.qho = qho;
            this.dlNum = dlNum;
            this.dmNumParent = dmNumParent;
            state = DLState.Waiting;

            //setup timers
            reConnect.AddEvent(new ElapsedEventHandler(reConnect_Tick));
            if (knownDLcount == -1)
            {
                knownDLcount = DownloadManager.dms[dmNumParent].downloaders.Count;
            }
            if (dlNum == 0)
            {
                reConnect.Interval = 10;
            }
            else if (knownDLcount > 800)
            {
                reConnect.Interval = GUID.rand.Next(100, 600000);
            }
            else if (knownDLcount > 400)
            {
                reConnect.Interval = GUID.rand.Next(100, 220000);
            }
            else if (knownDLcount > 200)
            {
                reConnect.Interval = GUID.rand.Next(100, 80000);
            }
            else if (knownDLcount > 80)
            {
                reConnect.Interval = GUID.rand.Next(100, 35000);
            }
            else if (knownDLcount > 40)
            {
                reConnect.Interval = GUID.rand.Next(100, 15000);
            }
            else
            {
                reConnect.Interval = GUID.rand.Next(10, 4000);
            }
            reConnect.Start();
            connectYet.AddEvent(new ElapsedEventHandler(connectYet_Tick));
            connectYet.Interval = 12000;
            connectYet.Start();
            deadYet.AddEvent(new ElapsedEventHandler(deadYet_Tick));
            deadYet.Interval = 20000;
            deadYet.Start();
        }

        public virtual void OnDeserialization(object dler)
        {
            if (state == DLState.CouldNotConnect || state == DLState.Connecting)
            {
                state = DLState.Waiting;
            }
            if (this.queueState != 0)
            {
                this.queueState = 0;
            }

            //NonSerialized stuff initializers
            reConnect = new GoodTimer();
            connectYet = new GoodTimer();
            deadYet = new GoodTimer();

            reConnect.AddEvent(new ElapsedEventHandler(reConnect_Tick));
            reConnect.Interval = 40000;
            count = 0;
            reConnect.Start();

            connectYet.AddEvent(new ElapsedEventHandler(connectYet_Tick));
            connectYet.Interval = 12000;

            deadYet.AddEvent(new ElapsedEventHandler(deadYet_Tick));
            deadYet.Interval = 20000;
            deadYet.Start();

            /*
             * we leave the streams alone, CreateFile will reassign them
             * we leave sock1 alone, it'll be reassigned in Reset()
             */
        }

        void reConnect_Tick(object sender, ElapsedEventArgs e)
        {
            if (DownloadManager.shutflag || !DownloadManager.dms[this.dmNumParent].active)
            {
                return;
            }
            reConnect.Interval = 1000;
            count--;

            //is it time to retry
            if (count <= 0 && state == DLState.Waiting)
            {
                //make sure we don't overflood our own bandwidth
                int activeCount = DownloadManager.dms[this.dmNumParent].GetActiveCount();
                bool ok = true;
                if (Configuration.settings.connectionType == EnumConnectionType.dialUp)
                {
                    if (activeCount > 4)
                    {
                        ok = false;
                    }
                }
                else if (Configuration.settings.connectionType == EnumConnectionType.cable)
                {
                    if (activeCount > 25)
                    {
                        ok = false;
                    }
                }
                else if (activeCount > 50)
                {
                    ok = false;
                }
                if (!ok)
                {
                    count = 50;
                    return;
                }

                count = 0;

                //CEH fool it re firewall for testing
                //potential push; we'll also send a push if we can't connect and the QH2 packet had NH children
                //if(IPfilter.Private(qho.ip))
                //{
                //    state = DLState.SentPush;
                //    Gnutella2.OPush.SendPush(this.qho);
                //}
                //else
                Reset();
            }
        }

        /// <summary>
        /// Connect to host.
        /// </summary>
        public void Reset()
        {
            if (DownloadManager.shutflag || !DownloadManager.dms[this.dmNumParent].active)
            {
                return;
            }

            //just a check
            if (state == DLState.Downloading || state == DLState.Connected)
            {
                return;
            }

            //if we're queued and we have to callback to a host to download, then it's ok; otherwise we can't be queued and run Reset()
            if (queueState != 0 && this.lastMessage != "Callbacking") //this.qho.networkType == NetworkType.EDonkey
            {
                return;
            }

            try
            {
                disconnectFired = false;

                //already connecting
                state = DLState.Connecting;
                this.receiveBuff = new byte[16384];

                sock1 = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                IPEndPoint endPoint;

                IPHostEntry ipHost = Dns.GetHostEntry(qho.ip);
                endPoint = new IPEndPoint(ipHost.AddressList[0], (int)qho.port);

                sock1.BeginConnect(endPoint, new AsyncCallback(OnConnect), sock1);
                connectYet.Start();
            }
            catch
            {
                Disconnect("Reset()");
            }
        }

        //OnSendData will start receiving if this flag is set to true
        bool beginReceive = false;

        /// <summary>
        /// Accept the incoming connection and start the download.
        /// </summary>
        public void ResetIncoming(Socket sockPushDL)
        {
            if (DownloadManager.shutflag || !DownloadManager.dms[this.dmNumParent].active)
            {
                return;
            }
            if (state != DLState.SentPush)
            {
                try
                {
                    if (sockPushDL != null)
                    {
                        if (sockPushDL.Connected)
                        {
                            sockPushDL.Shutdown(SocketShutdown.Both);
                        }
                        sockPushDL.Close();
                    }
                }
                catch
                {
                    System.Diagnostics.Debug.WriteLine("DLer ResetIncoming");
                }
                return;
            }
            try
            {
                disconnectFired = false;
                this.receiveBuff = new byte[16384];
                sock1 = sockPushDL;
                state = DLState.Connected;
                queueState = 0;
                beginReceive = true;

                DownloadManager.dms[this.dmNumParent].AvailableDownloader(this.dlNum);
            }
            catch
            {
                Disconnect("Downloader ResetIncoming");
            }
        }

        /// <summary>
        /// Copy the remote IP Address of this pushed download to allow a retry.
        /// </summary>
        public void RemoteIP()
        {
            string tempEnd = "";
            try
            {
                tempEnd = sock1.RemoteEndPoint.ToString();
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("Downloader RemoteIP 1");
            }
            int xPos = tempEnd.IndexOf(":");
            if (xPos == -1)
            {
                return;
            }
            try
            {
                string elIP = tempEnd.Substring(0, xPos);
                qho.ip = elIP;
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("Downloader RemoteIP 2");
            }
        }

        void OnConnect(IAsyncResult ar)
        {
            //make sure we're not already connected
            if (state != DLState.Connecting || DownloadManager.shutflag || !DownloadManager.dms[this.dmNumParent].active)
            {
                return;
            }
            try
            {
                Socket tmpSock = (Socket)ar.AsyncState;
                tmpSock.EndConnect(ar);
                connectYet.Stop();
                state = DLState.Connected;
                queueState = 0;

                beginReceive = true;

                //tell the DownloadManager
                DownloadManager.dms[this.dmNumParent].AvailableDownloader(this.dlNum);
            }
            catch
            {
                Disconnect("OnConnect");
            }
        }

        void OnReceiveData(IAsyncResult ar)
        {
            try
            {
                Socket tmpSock = (Socket)ar.AsyncState;
                int bytesRec = tmpSock.EndReceive(ar);
                Configuration.State.inTransferBandwidth += bytesRec;

                //do something with it
                DataReceived(bytesRec);

                //start receiving again
                if (bytesRec > 0)
                {
                    tmpSock.BeginReceive(receiveBuff, 0, receiveBuff.Length, SocketFlags.None, new AsyncCallback(OnReceiveData), tmpSock);
                }
            }
            catch
            {
                Disconnect("OnReceiveData");
            }
        }

        public void DataReceived(int bytesRec)
        {
            try
            {
                if (bytesRec > 0)
                {
                    //update stuff
                    receivedData = true;
                    DownloadManager.dms[this.dmNumParent].bytesRec += bytesRec;

                    //gnutella file data
                    if (state == DLState.Downloading)
                    {
                        if (curOffset == 0 && !this.dlflushed)
                        {
                            //we're in a potential handshake mode, so things can be inefficient

                            //append this data to tempBuf
                            byte[] tempMsg = new byte[bytesRec];
                            Array.Copy(receiveBuff, 0, tempMsg, 0, bytesRec);
                            tempBuf.AddRange(tempMsg);

                            //get what's in entire tempBuf
                            byte[] elBuf = new byte[tempBuf.Count];
                            tempBuf.CopyTo(elBuf);

                            string what = Encoding.ASCII.GetString(elBuf, 0, elBuf.Length);
                            if (what.IndexOf("\r\n\r\n") != -1)
                            {
                                //supports chat
                                int chatIndex = what.IndexOf("Chat: ") + 6;
                                if (chatIndex != 5)
                                {
                                    this.chatIP = what.Substring(chatIndex, what.IndexOf("\r", chatIndex) - chatIndex);
                                }

                                //vendor
                                if (this.qho.vendor.Length == 0)
                                {
                                    int vendIndex = what.IndexOf("Server: ") + 8;
                                    if (vendIndex != 7)
                                    {
                                        this.qho.vendor = what.Substring(vendIndex, what.IndexOf("\r", vendIndex) - vendIndex);
                                    }
                                }

                                //System.Diagnostics.Debug.WriteLine("Gnutella DL Header:\r\n" + what);
                                string first = what.Substring(0, what.IndexOf("\r\n")).ToLower();

                                //decide what to do
                                if (    first.IndexOf(" 200 ") != -1 ||
                                        first.IndexOf(" 206 ") != -1 ||
                                        first.IndexOf("ok") != -1 ||
                                        first.IndexOf("partial") != -1 ||
                                        what.ToLower().IndexOf("http 200 ok") != -1 )
                                {
                                    //it's all good
                                    int start = what.IndexOf("\r\n\r\n") + 4;

                                    //update our position; write to file
                                    DownloadManager.dms[this.dmNumParent].DLFlushData(startOffset + curOffset, receiveBuff, start, elBuf.Length - start, this.dlNum, ref curOffset);
                                }
                                else if (   first.IndexOf(" 503 ") != -1 ||
                                            first.IndexOf("busy") != -1 ||
                                            (   first.IndexOf("<html>") != -1 && 
                                                what.ToLower().IndexOf("server busy") != -1 )   )
                                {
                                    //server busy
                                    lastMessage = "Server Busy";
                                    Disconnect("server busy");
                                }
                                else if (   first.IndexOf(" 404 ") != -1 ||
                                            first.IndexOf("not found") != -1 ||
                                            (   first.IndexOf("<html>") != -1 &&
                                                what.ToLower().IndexOf("404 not found") != -1   )   )
                                {
                                    //file not found
                                    lastMessage = "File Not Found";
                                    Disconnect("file not found");
                                    state = DLState.CouldNotConnect;
                                }
                                else
                                {
                                    //unknown
                                    lastMessage = "Unknown Error";
                                    //System.Diagnostics.Debug.WriteLine("unknown error: " + what);
                                    Disconnect("unknown error");
                                }
                            }
                        }
                        else
                        {
                            //update our position; write to file
                            DownloadManager.dms[this.dmNumParent].DLFlushData(startOffset + curOffset, receiveBuff, 0, bytesRec, this.dlNum, ref curOffset);
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Disconnect("dler DataReceived");
                System.Diagnostics.Debug.WriteLine(e.Message);
                System.Diagnostics.Debug.WriteLine(e.StackTrace);
            }
        }

        /// <summary>
        /// Send a string to our host.
        /// </summary>
        public void SendString(string msg)
        {
            try
            {
                sock1.BeginSend(Encoding.ASCII.GetBytes(msg), 0, msg.Length, SocketFlags.None, new AsyncCallback(OnSendData), sock1);
            }
            catch
            {
                if (!DownloadManager.shutflag)
                {
                    Disconnect("SendString");
                }
            }
        }

        void OnSendData(IAsyncResult ar)
        {
            try
            {
                Socket tmpSock = (Socket)ar.AsyncState;
                int bytesSent = tmpSock.EndSend(ar);
                Configuration.State.outTransferBandwidth += bytesSent;
                if (beginReceive)
                {
                    beginReceive = false;
                    sock1.BeginReceive(receiveBuff, 0, receiveBuff.Length, SocketFlags.None, new AsyncCallback(OnReceiveData), sock1);
                }
            }
            catch
            {
                Disconnect("OnSendData");
            }
        }

        /// <summary>
        /// Fill the fileParts SortedList with acceptable offset values.
        /// It'll hold "up to and including" offsets.
        /// </summary>
        //void ParseFilePartArray(byte[] parts)
        //{
        //    if(parts.Length == 0)
        //    {
        //        this.fileParts.Clear();
        //        this.fileParts.Add((uint)0, qho.fileSize - 1);
        //    }
        //    else
        //    {
        //        bool startFound = false;
        //        uint start = 0;
        //        this.fileParts.Clear();
        //        for(uint x = 0; x < parts.Length; x++)
        //        {
        //            bool chunkDone = (parts[x] == 1);
        //            if(chunkDone && !startFound)
        //            {
        //                start = Configuration.State.eDonkeyPartSize * x;
        //                startFound = true;
        //            }
        //            else if(!chunkDone && startFound)
        //            {
        //                this.fileParts.Add(start, (x * Configuration.State.eDonkeyPartSize) - 1);
        //                startFound = false;
        //            }
        //        }
        //        if(startFound)
        //            this.fileParts.Add(start, qho.fileSize - 1);
        //    }
        //}

        public void Disconnect(string where)
        {
            if (DownloadManager.dms[this.dmNumParent] == null)
            {
                return;
            }
            lock (DownloadManager.dms[this.dmNumParent].endpoints)
            {
                if (disconnectFired == true)
                {
                    return;
                }
                disconnectFired = true;
                if (queueState != 0)
                {
                    queueState = 0;
                }
                if (this.lastMessage == "Queued")
                {
                    this.lastMessage = "";
                    queueState = -1;
                }

                try
                {
                    if (this.qho == null)
                    {
                        return;
                    }
                    connectYet.Stop();
                    this.dlflushed = false;
                    if (sock1 != null)
                    {
                        if (sock1.Connected)
                        {
                            sock1.Shutdown(SocketShutdown.Both);
                        }
                        sock1.Close();
                        sock1 = null;
                    }
                    //if(where == "cleaning up")
                    //{
                    //	System.Diagnostics.Debug.WriteLine("dl 1");
                    //}

                    //if(where == "cleaning up")
                    //{
                    //	System.Diagnostics.Debug.WriteLine("dl 2");
                    //}
                    this.receiveBuff = null;
                    tempBuf.Clear();
                    switch (state)
                    {
                        case DLState.Connected:
                            state = DLState.Waiting;
                            this.count = 40;
                            this.curOffset = 0;
                            this.startOffset = 0;
                            break;
                        case DLState.Connecting:
                            lastMessage = "";
                            state = DLState.CouldNotConnect;
                            break;
                        case DLState.CouldNotConnect:
                            break;
                        case DLState.Downloading:
                            state = DLState.Waiting;
                            //let the DownloadManager know of our little incident
                            DownloadManager.dms[this.dmNumParent].DLerDone(this.dlNum);
                            if (quickReconnect == true)
                            {
                                //now the count is 2 so that a reconnect spawns in 2 sec
                                quickReconnect = false;
                                this.count = 2;
                            }
                            else
                            {
                                this.count = 40;
                            }
                            this.curOffset = 0;
                            this.startOffset = 0;
                            break;
                        case DLState.SentPush:
                            break;
                        case DLState.Waiting:
                            break;
                    }
                }
                catch (Exception e)
                {
                    System.Diagnostics.Debug.WriteLine("Downloader Disconnect: " + e.Message + "\r\n" + e.Source + "\r\n" + e.StackTrace);
                }
            }
        }

        /// <summary>
        /// Start downloading the file at the specified offsets.
        /// </summary>
        public void StartTransfer(uint start, uint stop)
        {
            //setup the offsets; we subtract 1 because we want "up to and including" an offset
            if (stop == 0)
            {
                stop = qho.fileSize - 1;
            }
            else
            {
                stop = stop - 1;
            }

            //make sure if the host sent us a part list, we check to see if our offsets can fit
            if (this.fileParts.Count > 0)
            {
                bool foundPart = false;
                for (int x = 0; x < this.fileParts.Count; x++)
                {
                    //note that these are "up to and including" values
                    uint startoffset = (uint)fileParts.GetKey(x);
                    uint endoffset = (uint)fileParts.GetByIndex(x);

                    //check if the first offset is in it
                    if (start >= startoffset && start < endoffset)
                    {
                        if (stop <= endoffset)
                        {
                            foundPart = true;
                        }
                        else if (endoffset - start >= DownloadManager.boundary)
                        {
                            foundPart = true;
                            stop = endoffset;
                        }
                        else if (stop == qho.fileSize - 1)
                        {
                            foundPart = true;
                        }
                        break;
                    }
                }
                if (!foundPart)
                {
                    System.Diagnostics.Debug.WriteLine("no part found");
                    this.Disconnect("StartTransfer no part found");
                    return;
                }
            }

            //other offset related stuff
            string sStart = start.ToString();
            string sStop = stop.ToString();

            //where are we?
            this.startOffset = start;

            //curOffset holds the offset from the start offset, not the beginning of the file
            this.curOffset = 0;
            state = DLState.Downloading;

            if (qho.sha1sum.Length == 0)
            {
                Utils.Diag("g2 dl no sha1sum on dl #" + this.dlNum.ToString());
            }

            //headers
            string httpGet = "";
            httpGet += "GET /uri-res/N2R?" + qho.sha1sum + " HTTP/1.1\r\n";
            httpGet += "User-Agent: " + Configuration.userAgent + " " + Configuration.version + "\r\n";
            httpGet += "Chat: " + Configuration.settings.ipAddress + ":" + Configuration.settings.port.ToString() + "\r\n";
            httpGet += "Host: " + qho.ip + ":" + qho.port.ToString() + "\r\n";
            httpGet += "Connection: Keep-Alive\r\n";
            httpGet += "Range: bytes=" + sStart + "-" + sStop + "\r\n";
            httpGet += "\r\n";
            SendString(httpGet);
        }

        public void NullifyMe()
        {
            try
            {
                this.qho = null;
                if (sock1 != null)
                {
                    if (sock1.Connected)
                    {
                        sock1.Shutdown(SocketShutdown.Both);
                    }
                    sock1.Close();
                    sock1 = null;
                }

                reConnect.Stop();
                reConnect.RemoveEvent(new ElapsedEventHandler(reConnect_Tick));
                reConnect.Close();
                connectYet.Stop();
                connectYet.RemoveEvent(new ElapsedEventHandler(connectYet_Tick));
                connectYet.Close();
                deadYet.Stop();
                deadYet.RemoveEvent(new ElapsedEventHandler(deadYet_Tick));
                deadYet.Close();
            }
            catch (Exception e)
            {
                System.Diagnostics.Debug.WriteLine("DLer NullifyMe: " + e.Message);
            }
        }

        void connectYet_Tick(object sender, ElapsedEventArgs e)
        {
            Disconnect("Connection Timer");
            if (this.qho.ipepNH != null)
            {
                this.state = DLState.SentPush;
                Gnutella2.OPush.SendPush(this.qho);
            }
        }

        void deadYet_Tick(object sender, ElapsedEventArgs e)
        {
            //check if the download is still alive
            if (state == DLState.Downloading)
            {
                if (receivedData == false)
                {
                    Disconnect("deadYet timer");
                    return;
                }
            }
            receivedData = false;

            //also, let's take the opportunity to check if the queue is still alive as well
            if (queueState != 0)
            {
                //20 seconds * 24 = 480 seconds = 8 minutes (on average)
                //we'll handle things this way so we don't need another timer to take up resources
                int rndres = GUID.rand.Next(0, 24);
                if (rndres == 1)
                {
                    //make queue state request
                    //if(this.qho.networkType == NetworkType.EDonkey)
                    //{
                    //    EDonkey.Messages.QueueStateCheck(this);
                    //}
                }
            }
        }
    }
}
