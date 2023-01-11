using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace HoytSoft.Common.Net {

	public class DNS {
		#region Constants
		public const string
			LIB_DNSAPI = "dnsapi"
		;
		#endregion

		#region Enums
		private enum QueryOptions {
			DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE = 1,
			DNS_QUERY_BYPASS_CACHE = 8,
			DNS_QUERY_DONT_RESET_TTL_VALUES = 0x100000,
			DNS_QUERY_NO_HOSTS_FILE = 0x40,
			DNS_QUERY_NO_LOCAL_NAME = 0x20,
			DNS_QUERY_NO_NETBT = 0x80,
			DNS_QUERY_NO_RECURSION = 4,
			DNS_QUERY_NO_WIRE_QUERY = 0x10,
			DNS_QUERY_RESERVED = -16777216,
			DNS_QUERY_RETURN_MESSAGE = 0x200,
			DNS_QUERY_STANDARD = 0,
			DNS_QUERY_TREAT_AS_FQDN = 0x1000,
			DNS_QUERY_USE_TCP_ONLY = 2,
			DNS_QUERY_WIRE_ONLY = 0x100
		}

		private enum QueryTypes : int {
			DNS_TYPE_MX = 15
		}
		#endregion

		#region Structs
		[StructLayout(LayoutKind.Sequential)]
		private struct MXRecord {
			public IntPtr pNext;
			public string pName;
			public short wType;
			public short wDataLength;
			public int flags;
			public int dwTtl;
			public int dwReserved;
			public IntPtr pNameExchange;
			public short wPreference;
			public short Pad;
		}
		#endregion

		#region PInvoke Declarations
		[DllImport(LIB_DNSAPI, CharSet = CharSet.Unicode, EntryPoint="DnsQuery_W", SetLastError = true)]
		private static extern int DnsQuery([MarshalAs(UnmanagedType.VBByRefStr)]ref string pszName, QueryTypes wType, QueryOptions options, int aipServers, ref IntPtr ppQueryResults, int pReserved);

		[DllImport(LIB_DNSAPI, CharSet = CharSet.Auto, SetLastError = true)]
		private static extern void DnsRecordListFree(IntPtr pRecordList, int FreeType);
		#endregion

		#region Constructors
		public DNS() {
		}
		#endregion

		#region Public Static Methods
		public static string[] GetMXRecords(string domain) {

			IntPtr ptr1 = IntPtr.Zero;
			IntPtr ptr2 = IntPtr.Zero;
			MXRecord recMx;
			if (Environment.OSVersion.Platform != PlatformID.Win32NT)
				throw new NotSupportedException();

			List<MXRecord> lstMX = new List<MXRecord>(1);
			int num1 = DnsQuery(ref domain, QueryTypes.DNS_TYPE_MX, QueryOptions.DNS_QUERY_BYPASS_CACHE, 0, ref ptr1, 0);
			if (num1 != 0)
				throw new Win32Exception(num1);
			
			for (ptr2 = ptr1; !ptr2.Equals(IntPtr.Zero); ptr2 = recMx.pNext) {
				recMx = (MXRecord)Marshal.PtrToStructure(ptr2, typeof(MXRecord));
				if (recMx.wType == 15)
					lstMX.Add(recMx);
			}

			//Sort based on preference
			lstMX.Sort(new Comparison<MXRecord>(delegate(MXRecord r1, MXRecord r2) {
				if (r1.wPreference > r2.wPreference)
					return 1;
				else if (r1.wPreference < r2.wPreference)
					return -1;
				else
					return 0;
			}));

			//Return array
			string[] ret = new string[lstMX.Count];
			for (int i = 0; i < lstMX.Count; ++i)
				ret[i] = Marshal.PtrToStringAuto(lstMX[i].pNameExchange);

			DnsRecordListFree(ptr1, 0);

			return ret;
		}
		#endregion
	}
}
