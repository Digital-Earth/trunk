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

namespace FileScopeCore
{

	/// <summary>
	/// Class for storing the  stats for FileScope.
	/// </summary>
	public class State
	{

            public static volatile int timestamp = 50;
            public static volatile int trueSpeed = -1;				//"real" speed of connection
            public static volatile bool le = BitConverter.IsLittleEndian;				//little or big endian
			public static volatile int kbShared=0;				//kilobytes you're sharing
			public static volatile int filesShared=0;				//# of files you're sharing
			public static volatile bool everIncoming=false;			//accepted an incoming tcp connection?
			public static volatile bool udpIncoming=false;			//ever received incoming udp packets
			public static volatile int uploads=0;					//# of files you've uploaded
			public static volatile int uploadsNow2=0;				//# of files currently uploading
			public static int uploadsNow
			{
				get{return uploadsNow2;}
				set{uploadsNow2 = value; Configuration.GUIBridge.RefreshHomePageTransfers();}
			}
			public static volatile int downloadsNow2=0;			//# of files currently downloading
			public static int downloadsNow
			{
				get{return downloadsNow2;}
                set { downloadsNow2 = value; Configuration.GUIBridge.RefreshHomePageTransfers(); }
			}
			public static volatile int numChats=0;				//number of active chats
			public static volatile bool closing=false;				//are we closing FileScope?
			public static volatile bool opened=false;					//did we fully open FileScope?
			public static volatile int inNetworkBandwidth=0;		//b/s inward network bandwidth
			public static volatile int outNetworkBandwidth=0;		//b/s outward network bandwidth
            public static volatile int inTransferBandwidth = 0;		//b/s inward transfers bandwidth
			public static volatile int outTransferBandwidth=0;	//b/s outward transfers bandwidth

			public class Gnutella2Stats
			{
				public static volatile bool ultrapeer=false;			//are we an ultrapeer now or not?
				public static volatile int lastConnectionCount2=0;  //stores the number of connections
				public static int lastConnectionCount
				{
					get{return lastConnectionCount2;}
					set
					{
						bool sameCount = (lastConnectionCount2 == value);
						lastConnectionCount2 = value;
						if(!sameCount)
                            Configuration.GUIBridge.RefreshHomePageNetworks();
					}
				}
				public static volatile int numPI=0;
				public static volatile int numPO=0;
				public static volatile int numQ2=0;
				public static volatile int numQA=0;
				public static volatile int numQH2=0;
				public static volatile int numLNI=0;
				public static volatile int numKHL=0;
				public static volatile int numPUSH=0;
				public static volatile int numQHT=0;
				public static volatile int numQKR=0;
				public static volatile int numQKA=0;
			}

		public static void Reset()
		{
            trueSpeed = -1;
			uploads = 0;
			uploadsNow = 0;
			downloadsNow = 0;
			numChats = 0;
			closing = false;
			everIncoming = false;
			udpIncoming = false;
			timestamp = 50;
            le = BitConverter.IsLittleEndian;
            Gnutella2Stats.lastConnectionCount = 0;
            Gnutella2Stats.numPI = 0;
            Gnutella2Stats.numPO = 0;
            Gnutella2Stats.numLNI = 0;
            Gnutella2Stats.numKHL = 0;
            Gnutella2Stats.numQ2 = 0;
            Gnutella2Stats.numQA = 0;
            Gnutella2Stats.numQH2 = 0;
            Gnutella2Stats.numPUSH = 0;
            Gnutella2Stats.numQHT = 0;
            Gnutella2Stats.numQKR = 0;
            Gnutella2Stats.numQKA = 0;
		}
        /// <summary>
        /// Return the speed of this connection.
        /// </summary>
        public static int GetSpeed()
        {
            if (trueSpeed != -1)
                return trueSpeed;
            else
                switch (Configuration.settings.connectionType)
                {
                    case EnumConnectionType.dialUp:
                        return 5;
                    case EnumConnectionType.cable:
                        return 100;
                    case EnumConnectionType.t1:
                        return 1000;
                    case EnumConnectionType.t3:
                        return 50000;
                    default:
                        return 100;
                }
        }
	}
}
